option(WITH_URBAN "Build the Urban Terrain library" ON)

if (WITH_URBAN)
  add_global_definitions(-DURBAN_LIB)
  add_global_include_dir(src)
  add_srcs(
    src/prop_cost_hata.cpp
    src/prop_cost_hata.h
    src/prop_cost_wi.cpp
    src/prop_cost_wi.h
    src/prop_data.h
    src/prop_foliage.cpp
    src/prop_foliage.h
    src/prop_hata.cpp
    src/prop_hata.h
    src/prop_suburban.cpp
    src/prop_suburban.h
    src/prop_urban.cpp
    src/prop_urban.h
    src/prop_itu_r.cpp
    src/prop_itu_r.h)
  add_scenario_dir(urban)
  add_doxygen_inputs(src)
endif ()
