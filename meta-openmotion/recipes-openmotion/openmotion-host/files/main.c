#include <stdio.h>    // standard io for printfs and standard stream stuff
#include <stdlib.h>   // standard lib for macro flags like EXIT_SUCCESS and mallocs
#include <string.h>   // string manipulation tools like strcmp, strchr, and strcspn
#include <unistd.h>   // unix standard header containing read, write, and close system calls
#include <fcntl.h>    // file control definitions like O_RDWR and O_SYNC for opening devices
#include <termios.h>  // core linux termios structures and baudrate speed constants
#include <stdint.h>   // exact-width integer types like uint8_t and uint32_t for binary safety
#include <errno.h>    // global system error number tracking variables to debug OS failures

/* Protocol Constants */
#define CMD_MOVE              0x01  // byte header id that confirms a packet is a move instruction
#define STATUS_OK             0x55  // stm32 ack response code token meaning command accepted
#define STATUS_MOVE_COMPLETE  0x77  // notification byte sent when hardware timer finishes pulsing
#define ERR_INVALID_COMMAND   0x99  // stm32 rejection code if packet id isn't recognized
#define ERR_INVALID_PARAMETER 0xAA  // stm32 rejection code if variables fail boundary safety checks

/* Machine Geometry Configuration (Step 4.3 Constants) */
#define STEPS_PER_MM          80.0    // multiplier constant matching pitch lead and microstepping ratios
#define DEFAULT_FEEDRATE_MM_MIN 1200.0 // fallback speed profile used if an F code is missing in a row

/**
 * Prints the help documentation for the user interface.
 */
void print_help(const char *prog_name) {
    printf("\n📖 OpenMotion Host App Controller — Help Menu\n"); // print program identity
    printf("==================================================\n"); // visual border layout separator
    printf("Usage:\n"); // header for terminal input guide
    printf("  %s <serial_port> <gcode_file>     Run a G-code motion file.\n", prog_name); // show required arg format
    printf("  %s -h, --help                     Show this help screen.\n\n", prog_name); // help flag layout option
    printf("Example:\n"); // section label for standard demo run
    printf("  %s /dev/ttyACM0 test.gcode\n\n", prog_name); // real world terminal execution example string
    printf("Supported G-code Subset:\n"); // documentation of implemented subset features
    printf("  G1 : Linear Move (e.g., G1 X10.5 F600)\n"); // explains G1 token intent
    printf("  X  : Relative/Absolute distance in mm (Positive = Forward, Negative = Backward)\n"); // details X value range
    printf("  F  : Feedrate speed in mm/minute\n"); // details F value scale metric
    printf("==================================================\n\n"); // ending decorative separator block
}

/**
 * Configures a Linux file descriptor for raw serial communication (8N1, 115200).
 */
int configure_serial_port(int fd) {
    struct termios tty; // instantiate terminal configurations data structure on stack
    
    /* WHY TCGETATTR: Linux has hundreds of background serial hidden properties. We must read 
     * the existing system state structure into our local variable before changing specific bits, 
     * otherwise we would completely wipe out unrelated driver properties and break the link. */
    if (tcgetattr(fd, &tty) != 0) { // pull current operating system config states into tty struct
        fprintf(stderr, "Error from tcgetattr: %s\n", strerror(errno)); // output error if device rejected
        return -1; // escape configuration sequence with error return
    }

    /* WHY B115200: Sets the baudrate speed frequency clock on both the outgoing transmitter (tx) 
     * and incoming receiver (rx) hardware lines to exactly 115200 bits per second to line up with stm32 config. */
    cfsetospeed(&tty, B115200); // map output transmission speed attribute bit fields
    cfsetispeed(&tty, B115200); // map input receive speed attribute bit fields
    
    /* WHY CLOCAL & CREAD: 
     * CLOCAL means 'Local Line'. Setting this tells Linux to ignore old school hardware modem control lines (DTR/RTS). 
     * If left off, the OS will freeze and sleep wait for an external carrier detect signal before processing bytes! 
     * CREAD means 'Receiver Enable'. If this bit is not active, the serial chip physically stops capturing input bytes. */
    tty.c_cflag |= (CLOCAL | CREAD); // apply local override and enable receiver hardware paths
    
    /* WHY CLEAR PARENB, CSTOPB, & CSIZE:
     * ~PARENB clears parity generation. Turning this off gives us standard non-parity framing. 
     * ~CSTOPB sets 1 stop bit. Clearing it tells the system to only append one framing stop bit instead of two.
     * ~CSIZE wipes out the data width payload size configuration bit masks entirely so we can overwrite them. */
    tty.c_cflag &= ~PARENB; // disable parity tracking calculations bit for clean transmission payload
    tty.c_cflag &= ~CSTOPB; // enforce exactly 1 stop bit protocol rule across communication wire frames
    tty.c_cflag &= ~CSIZE; // clear historical character size bit field configurations masks
    
    /* WHY CS8: Sets the character frame bit payload width to exactly 8 bits. Combined with the 
     * rules above, this implements the industry standard '8N1' raw hardware formatting matrix profile. */
    tty.c_cflag |= CS8; // enforce strict 8 data bits packet sizing format boundaries

    /* WHY TURN OFF ICANON, ECHO, ECHOE, & ISIG (THE LOCAL MODES):
     * ~ICANON kills canonical mode. Turning it off enables RAW MODE. Normally Linux waits untill an enter key newline
     * character is typed before releasing data to an application. We need raw binary bytes instantly as they land!
     * ~ECHO & ~ECHOE stop automatic character echoing. If left on, Linux would automatically reflect every byte back 
     * to the stm32, creating infinite loop echo data noise packets on the rx line.
     * ~ISIG disables signal keys. If left on, a raw data byte matching a ctrl-c code would kill our application! */
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // turn off line buffering, terminal echoes, and system signals

    /* WHY TURN OFF IXON, IXOFF, & IXANY (THE INPUT MODES):
     * Disables software flow control (XON/XOFF handshaking). If left on, the Linux kernel scans incoming data bytes 
     * looking for flow control symbols. If your binary packet contains a number like 0x11 (XON) or 0x13 (XOFF), 
     * Linux will capture it and pause your whole app communication line indefinitely! */
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // stop software flow control handshaking completely from blocking streams

    /* WHY TURN OFF THE REST OF THE INPUT TRANSLATION FLAGS:
     * ~IGNBRK/BRKINT/PARMRK/ISTRIP: Stops the OS from stripping out bits or injecting markers if line breaks happen.
     * ~INLCR/IGNCR/ICRNL: Crucial! Normally, Linux automatically converts line breaks like carriage returns or newlines 
     * (\r to \n or \n to \r). If left on, any raw binary number in our packet that happens to equal 0x0A or 0x0D 
     * would be modified by the OS, corrupting our binary payload integers completely! */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // completely bypass incoming byte filters

    /* WHY TURN OFF OPOST:
     * Disables post-processing of output text characters. If left on, Linux automatically alters a lone \n byte into 
     * a carriage-return + newline combination (\r\n) before blasting it out. Disabling it guarantees raw binary output. */
    tty.c_oflag &= ~OPOST; // clear output processing maps so data buffer registers pass untouched

    /* WHY VMIN = 1 & VTIME = 0:
     * VMIN = 1 means read() will block and wait untill at least 1 single byte lands in the kernel buffer before unblocking.
     * VTIME = 0 specifies an infinite read timeout delay. The program will freeze cleanly on read lines without spinning 
     * the cpu in a heavy resource-wasting loop, waking up instantly the microsecond the stm32 replies. */
    tty.c_cc[VMIN] = 1; // force read actions to sleep block untill a character is physically retrieved
    tty.c_cc[VTIME] = 0; // disable tracking timeout intervals clock entirely

    /* WHY TCSETATTR & TCSANOW:
     * Writes our modified local tty config structure directly back into the Linux kernel serial driver. 
     * TCSANOW ensures that these strict configuration flag mutations take effect instantly right now. */
    if (tcsetattr(fd, TCSANOW, &tty) != 0) { // pass raw configurations down into system driver layers
        fprintf(stderr, "Error from tcsetattr: %s\n", strerror(errno)); // log failure string if driver rejects settings
        return -1; // drop out of sequence returning error flag state
    }
    return 0; // configuration completed sucessfully without errors
}

/**
 * Sends a single 10-byte raw protocol binary frame down the wire and blocks
 * waiting for both STATUS_OK (acceptance) and STATUS_MOVE_COMPLETE (done).
 */
int send_and_wait_move(int fd, uint8_t direction, uint32_t steps, uint32_t delay_us) {
    uint8_t packet[10]; // allocate 10 byte raw storage array on active stack memory
    packet[0] = CMD_MOVE; // save move command token constant inside zero index position
    packet[1] = direction; // assign active direction state flag directly to slot index one

    /* WHY BIT SHIFTS (>> 24, 16, 8): 
     * Microcontrollers and computers store numbers in multi-byte chunks, but serial lines can only send 
     * 1 byte (8 bits) at a time. We use bit shifts to chop up our 32-bit integers into 4 separate bytes, 
     * sending the Most Significant Byte (MSB) first to guarantee Big-Endian alignment for our parser. */
    packet[2] = (steps >> 24) & 0xFF; // push highest 8 bits of steps integer down to lowest block position
    packet[3] = (steps >> 16) & 0xFF; // push upper middle 8 bits of steps integer to target position
    packet[4] = (steps >> 8)  & 0xFF; // push lower middle 8 bits of steps integer down to track position
    packet[5] = steps         & 0xFF; // isolate absolute lowest 8 bits of raw steps integer variable

    packet[6] = (delay_us >> 24) & 0xFF; // slice highest 8 bits out of delay microsecond value
    packet[7] = (delay_us >> 16) & 0xFF; // slice upper middle 8 bits out of delay microsecond value
    packet[8] = (delay_us >> 8)  & 0xFF; // slice lower middle 8 bits out of delay microsecond value
    packet[9] = delay_us         & 0xFF; // isolate absolute lowest 8 bits out of delay microsecond value

    /* WHY TCFLUSH: While our motor was busy turning, the stm32 might have booted or dropped stray noise 
     * characters into the line. Calling tcflush clears out the operating system's background hardware buffers completely. 
     * This guarantees that when we read a response byte next, we get the exact fresh answer the stm32 generated 
     * right after our command, instead of a stale garbage byte from minutes ago. */
    tcflush(fd, TCIOFLUSH); // flush previous rx/tx old buffer trash before writing packet frame

    /* WHY WRITE: Sends the 10-byte data block over the serial file descriptor link. 
     * We track the return value to verify that all 10 bytes were accepted by the kernel stream layer. */
    if (write(fd, packet, 10) != 10) { // transmit packed array over low-level hardware line pipelines
        fprintf(stderr, "❌ Fail to write complete 10-byte packet.\n"); // log transmission dropped byte warnings
        return -1; // throw failure status tracker back to calling function loop
    }

    /* 1. Wait for instant ACK response from STM32 */
    uint8_t response = 0; // prepare local memory variable storage container for response code
    if (read(fd, &response, 1) <= 0) return -1; // execute blocking read call, sleeping loop untill response byte drops

    if (response == STATUS_OK) { // verify code token matches accepted confirmation identifier
        printf("  ↳ ✅ Command Accepted! Running steps...\n"); // log successful handshake validation to command line
    } else { // trigger code path if alternative rejection bytes are processed
        fprintf(stderr, "  ↳ ❌ STM32 Rejected Command! Code: 0x%02X\n", response); // print offending raw error hex code
        return -1; // return error exit code path immediately
    }

    /* 2. Block and wait for physical completion flag from STM32 */
    /* WHY A SECOND READ: This implements our step-and-wait sync protocol loop logic. The first read above was the instant ack 
     * telling us the stm32 got the command. This second read blocks our host app program on this line for seconds 
     * or minutes while the physical motor is turning, keeping everything perfectly synchronized. */
    if (read(fd, &response, 1) <= 0) return -1; // freeze execution path until motor finishes physical execution steps

    if (response == STATUS_MOVE_COMPLETE) { // trace if response code matches expected movement completed index
        printf("  ↳ 🎉 Move Completed! Motor is safe to command again.\n"); // log complete execution state confirmation
        return 0; // return successful step processing flag status
    } else { // hit trap fallback if unknown byte retrieved instead of complete indicator
        fprintf(stderr, "  ↳ ❌ Unexpected response while waiting for completion: 0x%02X\n", response); // output tracking anomaly alert
        return -1; // exit execution out with bad state flag tracker
    }
}

int main(int argc, char *argv[]) {
    /* WHY ARGC & ARGV: 
     * argc (argument count) holds the total number of items typed into the terminal command.
     * argv (argument vector) is an array of strings holding those exact inputs. 
     * argv[0] is always the program name itself (./host_app). */
    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) { // parse if help menu options selected
        print_help(argv[0]); // delegate logic to print layout specifications instructions manual
        return EXIT_SUCCESS; // stop program tracking cleanly with standard success status macro
    }

    /* Guard: Verify exact parameters provided */
    if (argc != 3) { // force script execution to look for exactly 2 arguments (port and filename)
        fprintf(stderr, "❌ Error: Invalid arguments.\n"); // alert user parameters mismatched formatting specs
        print_help(argv[0]); // print valid user options map rules guide
        return EXIT_FAILURE; // terminate tracking with error flag status code macro
    }

    const char *portname = argv[1]; // assign first option string index to serial link file tracking path
    const char *filename = argv[2]; // assign second option string location path directly to target gcode variable

    /* Open the targeted G-code file */
    FILE *gcode_file = fopen(filename, "r"); // request OS stream pointer reference handle to read text file instructions
    if (!gcode_file) { // verify valid file reference returned instead of an empty null system pointer
        fprintf(stderr, "❌ Error opening file '%s': %s\n", filename, strerror(errno)); // output system reason for open failure
        return EXIT_FAILURE; // fail run sequence since target file instruction list cannot be processed
    }

    /* Open the targeted Serial Port */
    printf("🔌 Opening connection to STM32 on %s...\n", portname); // echo target port tracking message to monitor
    
    /* WHY OPEN SYSTEM CALL: Opens our device node.
     * O_RDWR: Opens the serial link for reading and writing data streams.
     * O_NOCTTY: Stands for 'No Controlling Terminal'. Tells Linux this device cannot become a main keyboard terminal interface.
     * O_SYNC: Forces synchronous output operations, preventing Linux from caching data arrays in system RAM buffers. */
    int fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC); // request low-level raw file descriptor assignment index from OS kernel
    if (fd < 0) { // confirm non-negative integer tracking index returned properly by kernel manager
        fprintf(stderr, "❌ Error opening serial port %s: %s\n", portname, strerror(errno)); // output connection block failure logs
        fclose(gcode_file); // release open file handle stream context safely back to disk system
        return EXIT_FAILURE; // drop execution tracking state sequence with a failure code
    }

    if (configure_serial_port(fd) < 0) { // pass file descriptor pointer index to strip terminal filters down
        close(fd); // break open device tracking node references hooks safely before exiting
        fclose(gcode_file); // drop text file stream pipeline references out of active memory map
        return EXIT_FAILURE; // terminate execution path with an error report code
    }

    printf("📂 Reading and processing lines from: %s\n\n", filename); // log file streaming initialization sequence target

    char line[128]; // allocate static row text storage tracking buffer space array on active stack frame
    int line_number = 0; // initialize sequence line tracking index for debugging file anomalies logs
    float current_feedrate = DEFAULT_FEEDRATE_MM_MIN; // pre-load system velocity default metric baseline speed tracking

    /* Loop through the file line-by-line */
    while (fgets(line, sizeof(line), gcode_file)) { // ingest character rows until pointer tracking hits end of file limit
        line_number++; // increment row counting layout tracking index counter
        
        // Remove trailing newlines/spaces
        /* WHY STRCSPN: G-code lines end with \n (Linux) or \r\n (Windows). This function calculates the index 
         * where these characters appear and overwrites them with a null terminator '\0', cleaning up the string for safe token slicing. */
        line[strcspn(line, "\r\n")] = 0; // scrub row string to truncate trailing line feeds away from raw tokens

        /* Skip empty lines or comment lines starting with ';' */
        if (strlen(line) == 0 || line[0] == ';') { // trap blank lines or inline remarks notes fields rows
            continue; // jump tracking routine skip flow straight back to pull next text row record
        }

        /* Check if line starts with an active movement command "G1" */
        if (strncmp(line, "G1", 2) == 0) { // target active linear interpolation path moves by tracking line strings prefix matching
            float target_x = 0.0; // isolate local tracking memory for coordinate metrics capture
            int has_x = 0; // internal tracking boolean state tracker flag to evaluate parameter matching states

            /* Parse tokens on line using strstr */
            char *x_ptr = strchr(line, 'X'); // search across text array line tracking pointer string address for letter X token
            char *f_ptr = strchr(line, 'F'); // scan along text sequence matching address index offset location of letter F token

            /* WHY SSCANF(PTR + 1): strchr returns a pointer pointing directly to the letter 'X' or 'F' inside the line. 
             * By adding +1 to that pointer address, we jump right past the letter, leaving sscanf to look straight at the float 
             * number following it, extracting it into our variable. */
            if (f_ptr) { // identify if structural velocity parameter changes exist on active stream line
                sscanf(f_ptr + 1, "%f", &current_feedrate); // slice float string metrics data directly out into global tracking speed var
            }
            if (x_ptr) { // trace if absolute geometric location target variable was embedded in instruction string
                sscanf(x_ptr + 1, "%f", &target_x); // slice floating coordinate data string out into local target variable block
                has_x = 1; // set tracking condition state flag active to step into planning pipeline calculations
            }

            if (has_x) { // route path execution block if coordinates target parameters validated successfully
                printf("[%d] Interpreting: %s\n", line_number, line); // echo source instruction code tracing match to monitoring logs

                /* Motion Planning Math Calculations (Step 4.3) */
                uint8_t direction = (target_x >= 0) ? 1 : 0; // decode directional polarity assignment states based on coordinate value sign
                float absolute_mm = (target_x >= 0) ? target_x : -target_x; // remove sign structures to evaluate absolute distance lengths math metrics
                
                uint32_t total_steps = (uint32_t)(absolute_mm * STEPS_PER_MM); // multiply absolute length metric directly by physical driver ticks scale ratio

                /* Convert mm/min to step delay in microseconds:
                 * Step Frequency (Hz) = (Feedrate_mm_min / 60) * STEPS_PER_MM
                 * Delay_us = 1,000,000 / Step Frequency
                 */
                float steps_per_second = (current_feedrate / 60.0) * STEPS_PER_MM; // evaluate pulse clock count target densities per second intervals
                uint32_t delay_us = (uint32_t)(1000000.0 / steps_per_second); // divide microsecond scale block by pulse counts to extract timer delay profiles

                if (total_steps == 0) { // watch for calculation edge cases where sub-micron values slice away into null pulse tracking counts
                    printf("  ⚠️ Skipping line: Target rounded down to 0 physical steps.\n"); // log filter optimization flag pass indicator
                    continue; // abort planning sequence processing and skip instruction trace directly back to next stream fetch row
                }

                printf("  ⚙️ Planned: %u steps | Dir: %u | Delay: %u us\n", total_steps, direction, delay_us); // print calculated kinematic telemetry properties block

                /* Send down the wire and block until executed */
                if (send_and_wait_move(fd, direction, total_steps, delay_us) < 0) { // pass kinematics attributes straight down binary network serialization link
                    fprintf(stderr, "💥 Critical communication failure at line %d. Aborting execution.\n", line_number); // throw broken wire data pipeline error alerts
                    break; // break link processing stream loop entirely to shield machinery from executing out-of-sync operations
                }
            }
        } else { // line patterns fail to match subset operational rules standards variables layouts
            printf("[%d] Ignored unhandled command line: %s\n", line_number, line); // echo diagnostic notice tracking non-kinematic lines skipping configurations
        }
    }

    printf("\n🏁 Reached end of file. Closing stream execution smoothly.\n"); // finalize file processing run sequence tracking metrics summary log
    close(fd); // disconnect system file descriptor indexing links pointers tracking hooks back from device driver layers
    fclose(gcode_file); // flush file handle processing streams memory buffers completely back into native operational system kernel pools
    return EXIT_SUCCESS; // report successful program runtime status exit back to hosting environment system
}