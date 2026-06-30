SUMMARY = "Simple native C++ hello application"
LICENSE = "CLOSED"

# this is where the source file code for this recipe:
SRC_URI = "file://main.cpp"

# this is a declared variable called S, and we store inside it the work directory for this recipe
# note that each reipe has its own work directory, and inside this directory we will find the output for this recipe build
# alsi if we claen with  "bitbake -c clean <recipe_name>" --> this will remove the recipe work directory
S = "${WORKDIR}"


do_compile() {
	install -d ${WORKDIR}/student-output
	
	${CXX} ${CXXFLAGS} ${LDFLAGS} \
		${S}/main.cpp \
		-o ${WORKDIR}/student-output/hello-native
}


do_install() {
	install -d ${D}${bindir}
	install -m 0755 \
		${WORKDIR}/student-output/hello-native \
		${D}${bindir}/hello-native
}
