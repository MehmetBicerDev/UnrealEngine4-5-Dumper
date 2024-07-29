# Unreal Engine 4-5 SDK Dumper

## Overview
This project provides a C++ tool to dump the SDK for Unreal Engine 4 and 5. It simplifies the process of obtaining class and variable offsets within the engine.It also doesnt require you to update the engine offsets.

## Features
Easy-to-use SDK dumping by injecting the tool into the Unreal Engine process.
Simple initialization and offset retrieval functions.

## Getting Started
### Prerequisites
A compiled version of this project.
An injector tool to inject the compiled DLL into the Unreal Engine process.
### Usage
**Inject the DLL**:
Inject the compiled DLL into the Unreal Engine process using your preferred DLL injector.

**Initialize the SDK**:
Call the SDK::Init(); function in your main function to initialize the dumped SDK.

**Retrieve Offsets**:
```
uint64_t GetOffset(std::string ClassName, std::string VariableName) {
    return DumpedClasses.find(ClassName) != DumpedClasses.end() ?
        (DumpedClasses[ClassName]->Offsets.find(VariableName) != DumpedClasses[ClassName]->Offsets.end()
            ? DumpedClasses[ClassName]->Offsets[VariableName] : 0)
        : 0;
}
```

Example Usage:
Retrieve the offset for a specific variable in a class.
```
auto offset = GetOffset("UWorld", "PersistentLevel");
```

# Credits
Dumper7 for engine offsets updater.
