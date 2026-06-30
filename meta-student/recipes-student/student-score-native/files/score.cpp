#include <cstdlib>
#include <iomanip>
#include <iostream>


int main(int argc, char * argv[]) {

	if (argc != 4) {
		std::cerr << "Usage: student-score <quiz> <lab> <exam>" << std::endl;
		return 1;
	}
	
	const int quiz = std::atoi(argv[1]);
	const int lab  = std::atoi(argv[2]);
	const int exam = std::atoi(argv[3]);
	const double average = (quiz + lab + exam) / 3.0;

	std::cout << "Student Score Report" << std::endl;
	std::cout << "Quiz: " << quiz << std::endl;
	std::cout << "Lab: " << lab << std::endl;
	std::cout << "Exam: " << exam << std::endl;
	std::cout << std::fixed << std::setprecision(2);
	std::cout << "Average: " << average << std::endl;
	std::cout << "Result: " << (average >= 60.0 ? "PASS" : "FAIL") << std::endl;
	return 0;

}
