SUMMARY = "Native C++ student score report"
LICENSE = "CLOSED"

SRC_URI = "file://score.cpp"
S = "${WORKDIR}"

inherit native

QUIZ_SCORE ?= "80"
LAB_SCORE ?= "90"
EXAM_SCORE ?= "70"

do_compile(){
	install -d ${WORKDIR}/student-output
	
	${CXX} ${CXXFLAGS} ${LDFLAGS} \
		${S}/score.cpp \
		-o ${WORKDIR}/student-output/student-score
}

do_run() {
	${WORKDIR}/student-output/student-score \
		${QUIZ_SCORE} ${LAB_SCORE} ${EXAM_SCORE} \
		> ${WORKDIR}/student-output/score-result.txt
}

addtask run after do_compile before do_install

do_install() {
	install -d ${D}${bindir}
	install -m 0755 \
		${WORKDIR}/student-output/student-score \
		${D}${bindir}/student-score
}
