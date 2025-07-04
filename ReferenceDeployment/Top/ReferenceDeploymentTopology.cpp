// // ======================================================================
// // \title  ReferenceDeploymentTopology.cpp
// // \brief cpp file containing the topology instantiation code
// //
// // ======================================================================
// // Provides access to autocoded functions
// #include <ReferenceDeployment/Top/ReferenceDeploymentTopologyAc.hpp>
// // Note: Uncomment when using Svc:TlmPacketizer
// //#include <ReferenceDeployment/Top/ReferenceDeploymentPacketsAc.hpp>

// // Necessary project-specified types
// #include <Fw/Types/MallocAllocator.hpp>
// #include <Svc/FrameAccumulator/FrameDetector/FprimeFrameDetector.hpp>
// #include <ReferenceDeployment/Top/Ports_ComPacketQueueEnumAc.hpp>

// #include <Fw/Logger/Logger.hpp>

// // Used for 1Hz synthetic cycling
// #include <Os/Mutex.hpp>

// // Allows easy reference to objects in FPP/autocoder required namespaces
// using namespace ReferenceDeployment;

// // The reference topology uses a malloc-based allocator for components that need to allocate memory during the
// // initialization phase.
// Fw::MallocAllocator mallocator;

// // FprimeFrameDetector is used to configure the FrameAccumulator to detect F Prime frames
// Svc::FrameDetectors::FprimeFrameDetector frameDetector;

// Svc::ComQueue::QueueConfigurationTable configurationTable;

// // The reference topology divides the incoming clock signal (1Hz) into sub-signals: 1Hz, 1/2Hz, and 1/4Hz with 0 offset
// Svc::RateGroupDriver::DividerSet rateGroupDivisorsSet{{{1, 0}, {2, 0}, {4, 0}}};

// // Rate groups may supply a context token to each of the attached children whose purpose is set by the project. The
// // reference topology sets each token to zero as these contexts are unused in this project.
// U32 rateGroup1Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};
// U32 rateGroup2Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};
// U32 rateGroup3Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};

// // A number of constants are needed for construction of the topology. These are specified here.
// enum TopologyConstants {
//     CMD_SEQ_BUFFER_SIZE = 5 * 1024,
//     FILE_DOWNLINK_TIMEOUT = 1000,
//     FILE_DOWNLINK_COOLDOWN = 1000,
//     FILE_DOWNLINK_CYCLE_TIME = 1000,
//     FILE_DOWNLINK_FILE_QUEUE_DEPTH = 10,
//     HEALTH_WATCHDOG_CODE = 0x123,
//     COMM_PRIORITY = 100,
//     // bufferManager constants
//     FRAMER_BUFFER_SIZE = FW_MAX(FW_COM_BUFFER_MAX_SIZE, FW_FILE_BUFFER_MAX_SIZE) + Svc::FprimeProtocol::FrameHeader::SERIALIZED_SIZE + Svc::FprimeProtocol::FrameTrailer::SERIALIZED_SIZE,
//     FRAMER_BUFFER_COUNT = 30,
//     DEFRAMER_BUFFER_SIZE = FW_MAX(FW_COM_BUFFER_MAX_SIZE, FW_FILE_BUFFER_MAX_SIZE),
//     DEFRAMER_BUFFER_COUNT = 30,
//     COM_DRIVER_BUFFER_SIZE = 3000,
//     COM_DRIVER_BUFFER_COUNT = 30,
//     BUFFER_MANAGER_ID = 200
// };

// // Ping entries are autocoded, however; this code is not properly exported. Thus, it is copied here.
// Svc::Health::PingEntry pingEntries[] = {
//     // {PingEntries::ReferenceDeployment_blockDrv::WARN, PingEntries::ReferenceDeployment_blockDrv::FATAL, "blockDrv"},
//     {PingEntries::ReferenceDeployment_tlmSend::WARN, PingEntries::ReferenceDeployment_tlmSend::FATAL, "chanTlm"},
//     {PingEntries::ReferenceDeployment_cmdDisp::WARN, PingEntries::ReferenceDeployment_cmdDisp::FATAL, "cmdDisp"},
//     {PingEntries::ReferenceDeployment_cmdSeq::WARN, PingEntries::ReferenceDeployment_cmdSeq::FATAL, "cmdSeq"},
//     {PingEntries::ReferenceDeployment_eventLogger::WARN, PingEntries::ReferenceDeployment_eventLogger::FATAL, "eventLogger"},
//     {PingEntries::ReferenceDeployment_fileDownlink::WARN, PingEntries::ReferenceDeployment_fileDownlink::FATAL, "fileDownlink"},
//     {PingEntries::ReferenceDeployment_fileManager::WARN, PingEntries::ReferenceDeployment_fileManager::FATAL, "fileManager"},
//     {PingEntries::ReferenceDeployment_fileUplink::WARN, PingEntries::ReferenceDeployment_fileUplink::FATAL, "fileUplink"},
//     {PingEntries::ReferenceDeployment_prmDb::WARN, PingEntries::ReferenceDeployment_prmDb::FATAL, "prmDb"},
//     {PingEntries::ReferenceDeployment_rateGroup1::WARN, PingEntries::ReferenceDeployment_rateGroup1::FATAL, "rateGroup1"},
//     {PingEntries::ReferenceDeployment_rateGroup2::WARN, PingEntries::ReferenceDeployment_rateGroup2::FATAL, "rateGroup2"},
//     {PingEntries::ReferenceDeployment_rateGroup3::WARN, PingEntries::ReferenceDeployment_rateGroup3::FATAL, "rateGroup3"},
// };

// /**
//  * \brief configure/setup components in project-specific way
//  *
//  * This is a *helper* function which configures/sets up each component requiring project specific input. This includes
//  * allocating resources, passing-in arguments, etc. This function may be inlined into the topology setup function if
//  * desired, but is extracted here for clarity.
//  */
// void configureTopology(const TopologyState& state) {
//     // Buffer managers need a configured set of buckets and an allocator used to allocate memory for those buckets.
//     Svc::BufferManager::BufferBins bufferMgrBins;
//     memset(&bufferMgrBins, 0, sizeof(bufferMgrBins));
//     bufferMgrBins.bins[0].bufferSize = FRAMER_BUFFER_SIZE;
//     bufferMgrBins.bins[0].numBuffers = FRAMER_BUFFER_COUNT;
//     bufferMgrBins.bins[1].bufferSize = DEFRAMER_BUFFER_SIZE;
//     bufferMgrBins.bins[1].numBuffers = DEFRAMER_BUFFER_COUNT;
//     bufferMgrBins.bins[2].bufferSize = COM_DRIVER_BUFFER_SIZE;
//     bufferMgrBins.bins[2].numBuffers = COM_DRIVER_BUFFER_COUNT;

//     bufferMgrBins.bins[3].bufferSize = 16;      // Add a bin for small (e.g. 16-byte) buffers
//     bufferMgrBins.bins[3].numBuffers = 10;      // Adjust count as needed for your app
//     bufferManager.setup(BUFFER_MANAGER_ID, 0, mallocator, bufferMgrBins);

//     // Frame accumulator needs to be passed a frame detector (default F Prime frame detector)
//     frameAccumulator.configure(frameDetector, 1, mallocator, 2048);

//     // Command sequencer needs to allocate memory to hold contents of command sequences
//     cmdSeq.allocateBuffer(0, mallocator, CMD_SEQ_BUFFER_SIZE);

//     // Rate group driver needs a divisor list
//     rateGroupDriver.configure(rateGroupDivisorsSet);

//     // Rate groups require context arrays.
//     rateGroup1.configure(rateGroup1Context, FW_NUM_ARRAY_ELEMENTS(rateGroup1Context));
//     rateGroup2.configure(rateGroup2Context, FW_NUM_ARRAY_ELEMENTS(rateGroup2Context));
//     rateGroup3.configure(rateGroup3Context, FW_NUM_ARRAY_ELEMENTS(rateGroup3Context));

//     // File downlink requires some project-derived properties.
//     fileDownlink.configure(FILE_DOWNLINK_TIMEOUT, FILE_DOWNLINK_COOLDOWN, FILE_DOWNLINK_CYCLE_TIME,
//                            FILE_DOWNLINK_FILE_QUEUE_DEPTH);

//     // Parameter database is configured with a database file name, and that file must be initially read.
//     // prmDb.configure("PrmDb.dat");
//     // prmDb.readParamFile();

//     // Health is supplied a set of ping entires.
//     health.setPingEntries(pingEntries, FW_NUM_ARRAY_ELEMENTS(pingEntries), HEALTH_WATCHDOG_CODE);

//     // Note: Uncomment when using Svc:TlmPacketizer
//     // tlmSend.setPacketList(ReferenceDeploymentPacketsPkts, ReferenceDeploymentPacketsIgnore, 1);

//     // ComQueue configuration
//     // Events (highest-priority)
//     configurationTable.entries[Ports_ComPacketQueue::EVENTS].depth = 100;
//     configurationTable.entries[Ports_ComPacketQueue::EVENTS].priority = 0;
//     // Telemetry
//     configurationTable.entries[Ports_ComPacketQueue::TELEMETRY].depth = 500;
//     configurationTable.entries[Ports_ComPacketQueue::TELEMETRY].priority = 2;
//     // File Downlink (first entry after the ComPacket queues = NUM_CONSTANTS)
//     configurationTable.entries[Ports_ComPacketQueue::NUM_CONSTANTS].depth = 100;
//     configurationTable.entries[Ports_ComPacketQueue::NUM_CONSTANTS].priority = 1;

//     Fw::Logger::log("EVENTS Depth: %u\n", configurationTable.entries[Ports_ComPacketQueue::EVENTS].depth);
//     // Allocation identifier is 0 as the MallocAllocator discards it
//     comQueue.configure(configurationTable, 0, mallocator);
//     // if (state.hostname != nullptr && state.port != 0) {
//     //     comDriver.configure(state.hostname, state.port);
//     // }
// }

// // Public functions for use in main program are namespaced with deployment name ReferenceDeployment
// namespace ReferenceDeployment {
// void setupTopology(const TopologyState& state) {
//     // Autocoded initialization. Function provided by autocoder.
//     initComponents(state);
//     // Autocoded id setup. Function provided by autocoder.
//     setBaseIds();
//     // Autocoded connection wiring. Function provided by autocoder.
//     connectComponents();
//     // Autocoded configuration. Function provided by autocoder.
//     configComponents(state);
//     // Deployment-specific component configuration. Function provided above. May be inlined, if desired.
//     configureTopology(state);
//     // Autocoded command registration. Function provided by autocoder.
//     regCommands();
//     // Autocoded parameter loading. Function provided by autocoder.
//     loadParameters();
//     // Autocoded task kick-off (active components). Function provided by autocoder.
//     startTasks(state);
//     // Initialize socket communication if and only if there is a valid specification
//     // if (state.hostname != nullptr && state.port != 0) {
//     //     Os::TaskString name("ReceiveTask");
//     //     // Uplink is configured for receive so a socket task is started
//     //     comDriver.start(name, COMM_PRIORITY, Default::STACK_SIZE);
//     // }
//     comDriver.configure(state.uartDevice, state.baudRate);

//     clockSource.start();
// }

// // Variables used for cycle simulation
// Os::Mutex cycleLock;
// volatile bool cycleFlag = true;

// void startSimulatedCycle(Fw::TimeInterval interval) {
//     clockSource.configure(interval.getUSeconds()/1000);
    
//     cycleLock.lock();
//     bool cycling = cycleFlag;
//     cycleLock.unLock();

//     // Main loop
//     while (cycling) {
//         // ReferenceDeployment::blockDrv.callIsr();
//         // Os::Task::delay(interval);
//         clockSource.cycle();

//         cycleLock.lock();
//         cycling = cycleFlag;
//         cycleLock.unLock();
//     }
// }

// void stopSimulatedCycle() {
//     cycleLock.lock();
//     cycleFlag = false;
//     cycleLock.unLock();
// }

// void teardownTopology(const TopologyState& state) {
//     // Autocoded (active component) task clean-up. Functions provided by topology autocoder.
//     stopTasks(state);
//     freeThreads(state);

//     // Other task clean-up.
//     // comDriver.stop();
//     // (void)comDriver.join();

//     // Resource deallocation
//     cmdSeq.deallocateBuffer(mallocator);
//     bufferManager.cleanup();
// }
// };  // namespace ReferenceDeployment

// ======================================================================
// \title  ReferenceDeploymentTopology.cpp
// \brief cpp file containing the topology instantiation code
//
// ======================================================================
// Provides access to autocoded functions
#include <ReferenceDeployment/Top/ReferenceDeploymentTopologyAc.hpp>
// Note: Uncomment when using Svc:TlmPacketizer
//#include <fprime-zephyr-reference/ReferenceDeployment/Top/ReferenceDeploymentPacketsAc.hpp>

// Necessary project-specified types
#include <Fw/Types/MallocAllocator.hpp>
#include <Svc/FrameAccumulator/FrameDetector/FprimeFrameDetector.hpp>
#include <ReferenceDeployment/Top/Ports_ComPacketQueueEnumAc.hpp>

// Used for 1Hz synthetic cycling
#include <Os/Mutex.hpp>
#include <Fw/Logger/Logger.hpp>

#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// Allows easy reference to objects in FPP/autocoder required namespaces
using namespace ReferenceDeployment;

// The reference topology uses a malloc-based allocator for components that need to allocate memory during the
// initialization phase.
Fw::MallocAllocator mallocator;

// FprimeFrameDetector is used to configure the FrameAccumulator to detect F Prime frames
Svc::FrameDetectors::FprimeFrameDetector frameDetector;

Svc::ComQueue::QueueConfigurationTable configurationTable;

// The reference topology divides the incoming clock signal (10Hz) into sub-signals: 10Hz, and 1Hz
Svc::RateGroupDriver::DividerSet rateGroupDivisorsSet{{{10, 0}, {100, 0}}};

// Rate groups may supply a context token to each of the attached children whose purpose is set by the project. The
// reference topology sets each token to zero as these contexts are unused in this project.
U32 rateGroup1Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};
U32 rateGroup2Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};

// A number of constants are needed for construction of the topology. These are specified here.
enum TopologyConstants {
    // CMD_SEQ_BUFFER_SIZE = 5 * 1024,
    // HEALTH_WATCHDOG_CODE = 0x123,
    // COMM_PRIORITY = 100,
    //     FILE_DOWNLINK_TIMEOUT = 1000,
    // FILE_DOWNLINK_COOLDOWN = 1000,
    // FILE_DOWNLINK_CYCLE_TIME = 1000,
    // FILE_DOWNLINK_FILE_QUEUE_DEPTH = 10,
    // bufferManager constants
    FRAMER_BUFFER_SIZE = FW_MAX(FW_COM_BUFFER_MAX_SIZE, FW_FILE_BUFFER_MAX_SIZE) + Svc::FprimeProtocol::FrameHeader::SERIALIZED_SIZE + Svc::FprimeProtocol::FrameTrailer::SERIALIZED_SIZE,
    FRAMER_BUFFER_COUNT = 1, // ComQ/Com protocol
    DEFRAMER_BUFFER_SIZE = FW_MAX(FW_COM_BUFFER_MAX_SIZE, FW_FILE_BUFFER_MAX_SIZE),
    DEFRAMER_BUFFER_COUNT = 5,
    COM_DRIVER_BUFFER_SIZE = FW_MAX(FW_COM_BUFFER_MAX_SIZE, FW_FILE_BUFFER_MAX_SIZE) + Svc::FprimeProtocol::FrameHeader::SERIALIZED_SIZE + Svc::FprimeProtocol::FrameTrailer::SERIALIZED_SIZE,
    COM_DRIVER_BUFFER_COUNT = 5,
    BUFFER_MANAGER_ID = 200
};

/**
 * \brief configure/setup components in project-specific way
 *
 * This is a *helper* function which configures/sets up each component requiring project specific input. This includes
 * allocating resources, passing-in arguments, etc. This function may be inlined into the topology setup function if
 * desired, but is extracted here for clarity.
 */
void configureTopology(const TopologyState& state) {
    // Buffer managers need a configured set of buckets and an allocator used to allocate memory for those buckets.
    Svc::BufferManager::BufferBins bufferMgrBins;
    memset(&bufferMgrBins, 0, sizeof(bufferMgrBins));
    bufferMgrBins.bins[0].bufferSize = FRAMER_BUFFER_SIZE;
    bufferMgrBins.bins[0].numBuffers = FRAMER_BUFFER_COUNT;
    bufferMgrBins.bins[1].bufferSize = DEFRAMER_BUFFER_SIZE;
    bufferMgrBins.bins[1].numBuffers = DEFRAMER_BUFFER_COUNT;
    bufferMgrBins.bins[2].bufferSize = COM_DRIVER_BUFFER_SIZE;
    bufferMgrBins.bins[2].numBuffers = COM_DRIVER_BUFFER_COUNT;
    bufferManager.setup(BUFFER_MANAGER_ID, 0, mallocator, bufferMgrBins);

    // Frame accumulator needs to be passed a frame detector (default F Prime frame detector)
    frameAccumulator.configure(frameDetector, 1, mallocator, 2048);

    // Command sequencer needs to allocate memory to hold contents of command sequences
    // cmdSeq.allocateBuffer(0, mallocator, CMD_SEQ_BUFFER_SIZE);

    // Rate group driver needs a divisor list
    rateGroupDriver.configure(rateGroupDivisorsSet);

    // Rate groups require context arrays.
    rateGroup1.configure(rateGroup1Context, FW_NUM_ARRAY_ELEMENTS(rateGroup1Context));
    rateGroup2.configure(rateGroup2Context, FW_NUM_ARRAY_ELEMENTS(rateGroup2Context));

     // File downlink requires some project-derived properties.
    // fileDownlink.configure(FILE_DOWNLINK_TIMEOUT, FILE_DOWNLINK_COOLDOWN, FILE_DOWNLINK_CYCLE_TIME,
    //                        FILE_DOWNLINK_FILE_QUEUE_DEPTH);

    // Health is supplied a set of ping entires.
    // health.setPingEntries(ConfigObjects::ReferenceDeployment_health::pingEntries,
                        //   ConfigObjects::ReferenceDeployment_health::NUM_PING_ENTRIES, HEALTH_WATCHDOG_CODE);

    // Note: Uncomment when using Svc:TlmPacketizer
    // tlmSend.setPacketList(ReferenceDeploymentPacketsPkts, ReferenceDeploymentPacketsIgnore, 1);

    // ComQueue configuration
    // Events (highest-priority)
    configurationTable.entries[Ports_ComPacketQueue::EVENTS].depth = 5;
    configurationTable.entries[Ports_ComPacketQueue::EVENTS].priority = 0;
    // Telemetry
    configurationTable.entries[Ports_ComPacketQueue::TELEMETRY].depth = 5;
    configurationTable.entries[Ports_ComPacketQueue::TELEMETRY].priority = 2;
    // File Downlink (first entry after the ComPacket queues = NUM_CONSTANTS)
    configurationTable.entries[Ports_ComPacketQueue::NUM_CONSTANTS].depth = 1;
    configurationTable.entries[Ports_ComPacketQueue::NUM_CONSTANTS].priority = 1;
    // Allocation identifier is 0 as the MallocAllocator discards it
    comQueue.configure(configurationTable, 0, mallocator);
    clockSource.configure(10); // 100 Hz core clock

    Os::File::Status status =
        gpioDriver.open(::led, Zephyr::ZephyrGpioDriver::GpioConfiguration::OUT);
    if (status != Os::File::Status::OP_OK) {
        Fw::Logger::log("[ERROR] Failed to open GPIO pin\n");
    }

}

// Public functions for use in main program are namespaced with deployment name ReferenceDeployment
namespace ReferenceDeployment {
void setupTopology(const TopologyState& state) {
    // Autocoded initialization. Function provided by autocoder.
    initComponents(state);
    // Autocoded id setup. Function provided by autocoder.
    setBaseIds();
    // Autocoded connection wiring. Function provided by autocoder.
    connectComponents();
    // Autocoded configuration. Function provided by autocoder.
    configComponents(state);
    // Deployment-specific component configuration. Function provided above. May be inlined, if desired.
    configureTopology(state);
    // Autocoded command registration. Function provided by autocoder.
    regCommands();
    // Autocoded parameter loading. Function provided by autocoder.
    loadParameters();
    // Autocoded task kick-off (active components). Function provided by autocoder.
    startTasks(state);
    // Uplink is configured for receive so a socket task is started
    comDriver.configure(state.uartDevice, state.baudRate);
    // Start rate groups
    clockSource.start();
}

void teardownTopology(const TopologyState& state) {
    // Autocoded (active component) task clean-up. Functions provided by topology autocoder.
    stopTasks(state);
    freeThreads(state);
    bufferManager.cleanup();
}
};  // namespace ReferenceDeployment
