SUMMARY = "Bootlin labs games package group"
DESCRIPTION = "Custom package group to group game-related recipes"

# inherit the class packagegroup
inherit packagegroup

RDEPENDS:${PN} = " \
    ninvaders \
"

