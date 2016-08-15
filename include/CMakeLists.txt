include(CheckCXXSourceCompiles) # To check the namespace for unordered_map
include(CheckCXXSymbolExists)

set(CMAKE_REQUIRED_FLAGS ${CMAKE_CXX11_EXTENSION_COMPILE_OPTION})

check_cxx_source_compiles(
"#include <unordered_map>

int main() {
    std::unordered_map<int, int> m;
    return m.size();
}"
    HAVE_STD_UNORDERED_MAP)

check_cxx_source_compiles(
"#include <tr1/unordered_map>

int main() {
    std::tr1::unordered_map<int, int> m;
    return m.size();
}"
    HAVE_STD_TR1_UNORDERED_MAP)
configure_file(unordered_map_config.h.in unordered_map_config.h)

check_cxx_symbol_exists(llround "cmath;cfloat" HAVE_LLROUND)
# check_cxx_symbol_exists doesn't accept scoped symbols so use our
# own test source for std::llround
check_cxx_source_compiles(
"#include <cmath>
#include <cfloat>

using std::llround;

int main() {
    return int(llround(0.3f));
}"
    HAVE_STD_LLROUND)
configure_file(clockround.h.in clockround.h)

add_global_include_dir_absolute(${CMAKE_CURRENT_BINARY_DIR})

add_global_include_dir(.)
add_srcs(
    3dmath.h
    antenna.h
    api.h
    application.h
    applink.h
    app_util.h
    buffer.h
    Cache.h
    calendar.h
    cellular.h
    circularbuffer.h
    clock.h
    context.h
    ControlMessages.h
    coordinates.h
    dynamic.h
    dynamic_vars.h
    external.h
    external_socket.h
    external_util.h
    fileio.h
    geometry.h
    gestalt.h
    gui.h
    if_queue.h
    if_scheduler.h
    #jlm.h  
    #jlm_observer_example.h  
    #jlm_observer.h
    library_info.h
    list.h
    mac.h
    main.h
    mapping.h
    memory.h
    message.h
    mobility.h
    network.h
    node.h
    parallel.h
    parallel_mpi.h
    partition.h
    phy_connectivity.h
    phy.h
    phy_sync.h
    product_info.h
    propagation.h
    prop_mimo.h
    qualnet_error.h
    qualnet_mutex.h
    random.h
    sched_calendar.h
    sched_splaytree.h
    sched_std_library.h
    scheduler.h
    scheduler_types.h
    simplesplay.h
    sliding_win.h
    splaytree.h
    stats_app.h
    stats_global.h
    stats.h
    stats_mac.h
    stats_net.h
    stats_phy.h
    stats_queue.h
    stats_transport.h
    tcpapps.h
    terrain.h
    timer_manager.h
    trace.h
    transport.h
    types.h
    UrbanCache.h
    user.h
    util_atomic.h
    util_interlock.h
    util_mini_matrix.h
    util_eigen_matrix.h
    util_phase.h
    WallClock.h
    weather.h
    proc-base.h
    proc-datamodel.h
    proc-dbw-cs.h
    proc-dbw-cs-impl.h
    proc-dbw-ds.h
    proc-dbw-ds-impl.h
    proc-dbw.h
    proc-dbw-node-kernel.h
    proc-nexus.h
    proc-process.h
    proc-stats-db-controller.h
    mimo_tgn_models.h
)
if (IS_EXATA)
    add_srcs(socketlayer_api.h socketlayer.h)
endif ()
if (UNIX)
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        add_srcs(osdeps/linux/i386_atomic.h)
    else ()
        add_srcs(osdeps/linux/x86-64_atomic.h)
    endif ()
elseif (WIN32)
    add_srcs(osdeps/windows/i386_atomic.h)
endif ()

if (UNIX)
    set(PORTABLETHREADS_EXCLUDE win32)
elseif (WIN32)
    set(PORTABLETHREADS_EXCLUDE unix)
endif ()

if (IS_EXATA)
    add_srcs(
        socketlayer_api.h
        socketlayer.h
    )
endif ()

add_doxygen_inputs(. osdeps/linux osdeps/windows)

if (NOT IS_EXATA)
    add_doxygen_excludes(socketlayer_api.h socketlayer.h)
endif ()
