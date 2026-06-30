SUMMARY = "Embedded Network Telemetry Gateway using cpp-httplib"
HOMEPAGE = "https://github.com/yhirose/cpp-httplib"
LICENSE = "CLOSED"


# fetch the HTTp form github and pull the custom logic files from PC

SRC_URI = "git://github.com/yhirose/cpp-httplib.git;protocol=https;branch=master \
           file://main.cpp \
           file://sender.cpp \
           file://sender.hpp \
           file://Makefile"

# a stable check commit hash from the repo, we use SRCREV to control which commit we will use, 
# if we did not specify it choose the most recent one by defualt
SRCREV = "v0.18.1"

# we store the recipe work directory in out variable S,Tell Yocto to move to the cloned git workspace 
S = "${WORKDIR}/git"

do_compile() {
	# copy our logic into the libarary folder
	cp ${WORKDIR}/main.cpp ${S}/
	cp ${WORKDIR}/sender.cpp ${S}/
	cp ${WORKDIR}/sender.hpp ${S}/
	cp ${WORKDIR}/Makefile ${S}/
	
	# Run the Makefile using the yocto target cross-compiler tools:
	oe_runmake
}

do_check() {
	bbplain "--- Executing Embedded Quality Gate Check ---"
	if [ ! -f "${S}/iot-gateway" ]; then
		bberror "QA failure: 'iot-gateway' executable was not built successfully!"
		exit 1
	fi 
	
	# verify that our binary contains network string dependencies
	if ! grep -q "httplib" ${S}/sender.cpp; then 
		bberror "QA failure: Source integrity error. Missing httplib hooks."
		exit 1
	fi 
	bbplain "QA Pass: Architecture and dependencies validated."
}

addtask check after do_compile before do_install


do_install() {
    # Use our Makefile's install instructions to copy the binary into target /usr/bin
    oe_runmake install DESTDIR=${D}
}

# Add the binary package to the final root filesystem target output
FILES:${PN} = "/usr/bin/iot-gateway"
