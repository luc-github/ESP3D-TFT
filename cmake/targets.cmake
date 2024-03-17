#global drivers
set(EXTRA_COMPONENT_DIRS hardware/common/components)

# all targets
file(GLOB TARGET_FILES "cmake/targets/*.cmake")
foreach(TARGET_FILE ${TARGET_FILES})
  include(${TARGET_FILE})
endforeach()