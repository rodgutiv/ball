### list all filenames of the directory here ###
SET(SOURCES_LIST
	AStarBondOrderStrategy.C
	bondOrderAssignment.C
	bondOrderAssignmentStrategy.C
	branchAndBoundBondOrderStrategy.C
#	DPBondAssignment.C
	KGreedyBondOrderStrategy.C
	partialBondOrderAssignment.C
)

IF (BALL_HAS_LPSOLVE)
	LIST(APPEND SOURCES_LIST ILPBondOrderStrategy.C)
ENDIF()

ADD_BALL_SOURCES("STRUCTURE/BONDORDERS" "${SOURCES_LIST}")