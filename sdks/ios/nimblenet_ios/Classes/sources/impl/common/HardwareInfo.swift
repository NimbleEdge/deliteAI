/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Foundation
import UIKit
import Darwin
import MachO
import SystemConfiguration
import CoreTelephony


@objc public class HardwareInfo:NSObject {
    
    private var loadPrevious = host_cpu_load_info()
    
    override public init() {}
    
    private static func getInternalDeviceId() -> String? {
        guard let appName = Bundle.main.infoDictionary?["CFBundleName"] as? String else {
            return nil
        }
        guard var appBundleIdentifier = Bundle.main.bundleIdentifier as String? else {
            return nil
        }
        
        appBundleIdentifier += "NE"
        
        do {
            if let retrievedValue = try KeychainManager.retrieve(appBundleIdentifier: appBundleIdentifier, appName: appName) {
                return retrievedValue
            }
        } catch {
            return nil
        }
        
        guard let newUUID = UIDevice.current.identifierForVendor?.uuidString else {
            return nil
        }
        
        let status = KeychainManager.save(appBundleIdentifier: appBundleIdentifier, appName: appName, password: newUUID)
        if(status == true){
            return newUUID
        }
        return nil
    }
    
    @objc public static func setInternalDeviceIdInConfig(configJsonString: String) -> String? {
        guard let jsonData = configJsonString.data(using: .utf8) else {
            return nil
        }
        
        do {
            var jsonObject = try JSONSerialization.jsonObject(with: jsonData, options: [])
            
            if var dict = jsonObject as? [String: Any] {
                dict["internalDeviceId"] = getInternalDeviceId()
                jsonObject = dict
            }
            
            let updatedJsonData = try JSONSerialization.data(withJSONObject: jsonObject, options: [])
            return String(data: updatedJsonData, encoding: .utf8)
        } catch {
            return nil
        }
    }
    
    
    public func getBatteryInfo() -> (batteryLevel: Float, isCharging: Bool) {
        
        // Remember current battery monitoring setting to reset it after checking.
        let userBatteryMonitoringSetting = UIDevice.current.isBatteryMonitoringEnabled
        
        defer {
            UIDevice.current.isBatteryMonitoringEnabled = userBatteryMonitoringSetting
        }
        
        UIDevice.current.isBatteryMonitoringEnabled = true
        
        let batteryLevel = UIDevice.current.batteryLevel == -1 ? -1 : UIDevice.current.batteryLevel * 100
        
        return (batteryLevel: batteryLevel, isCharging: UIDevice.current.batteryState == .charging)
    }
    
    func hostCPULoadInfo() -> host_cpu_load_info? {
        let HOST_CPU_LOAD_INFO_COUNT = MemoryLayout<host_cpu_load_info>.stride/MemoryLayout<integer_t>.stride
        var size = mach_msg_type_number_t(HOST_CPU_LOAD_INFO_COUNT)
        var cpuLoadInfo = host_cpu_load_info()
        
        let result = withUnsafeMutablePointer(to: &cpuLoadInfo) {
            $0.withMemoryRebound(to: integer_t.self, capacity: HOST_CPU_LOAD_INFO_COUNT) {
                host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, $0, &size)
            }
        }
        if result != KERN_SUCCESS{
            print("Error  - \(#file): \(#function) - kern_result_t = \(result)")
            return nil
        }
        return cpuLoadInfo
    }
    
    public func getCpuUsage() -> (system: Double, user: Double, idle : Double, nice: Double)? {
        guard let load = hostCPULoadInfo() else {
            return nil
        }
        
        let usrDiff: Double = Double(load.cpu_ticks.0 - loadPrevious.cpu_ticks.0);
        let systDiff = Double(load.cpu_ticks.1 - loadPrevious.cpu_ticks.1);
        let idleDiff = Double(load.cpu_ticks.2 - loadPrevious.cpu_ticks.2);
        let niceDiff = Double(load.cpu_ticks.3 - loadPrevious.cpu_ticks.3);
        
        let totalTicks = usrDiff + systDiff + idleDiff + niceDiff
        print("Total ticks is ", totalTicks);
        let sys = systDiff / totalTicks * 100.0
        let usr = usrDiff / totalTicks * 100.0
        let idle = idleDiff / totalTicks * 100.0
        let nice = niceDiff / totalTicks * 100.0
        
        return (sys, usr, idle, nice);
    }
    
    @available(iOS 11.0, *)
    public func getDeviceThermalState() -> String {
        switch ProcessInfo.processInfo.thermalState {
        case .nominal:
            return "nominal"
        case .fair:
            return "fair"
        case .serious:
            return "serious"
        case .critical:
            return "critical"
        @unknown default:
            return "unknown"
        }
    }
    
    // https://stackoverflow.com/a/64640842
    public func memoryUsage() -> (used: UInt64, total: UInt64) {
        var taskInfo = task_vm_info_data_t()
        var count = mach_msg_type_number_t(MemoryLayout<task_vm_info>.size) / 4
        let result: kern_return_t = withUnsafeMutablePointer(to: &taskInfo) {
            $0.withMemoryRebound(to: integer_t.self, capacity: 1) {
                task_info(mach_task_self_, task_flavor_t(TASK_VM_INFO), $0, &count)
            }
        }
        
        var used: UInt64 = 0
        if result == KERN_SUCCESS {
            used = UInt64(taskInfo.phys_footprint)
        }
        
        let total = ProcessInfo.processInfo.physicalMemory
        return (used, total)
    }
    
    @available(iOS 11.0, *)
    public func getGPUMemoryUsage() -> Int? {
        guard let device = MTLCreateSystemDefaultDevice() else {
            return nil
        }
        
        return device.currentAllocatedSize
    }
    
    public func getHardwareInfo() -> [String: String] {
        return [
            "os": UIDevice.current.systemVersion,
            "cpu_model": UIDevice.current.getCPUName(),
            "cpu_arch": getArchitecture(),
            "clock_speed": UIDevice.current.getCPUSpeed(),
            "cpu_num": String(ProcessInfo.processInfo.processorCount),
            "gpu": getGPUname()
        ]
    }
    
    private func getArchitecture() -> String {
        guard let info = NXGetLocalArchInfo() else {
            return "unknown"
        }
        
        return String(cString: info.pointee.description)
    }
    
    private func getGPUname() -> String {
        guard let device = MTLCreateSystemDefaultDevice() else {
            return "unknown"
        }
        
        return device.name
    }
    
    public func getConnectionType() -> String {
        guard let reachability = SCNetworkReachabilityCreateWithName(kCFAllocatorDefault, "www.google.com") else {
            return "NO INTERNET"
        }
        
        var flags = SCNetworkReachabilityFlags()
        SCNetworkReachabilityGetFlags(reachability, &flags)
        
        let isReachable = flags.contains(.reachable)
        let isWWAN = flags.contains(.isWWAN)
        
        if isReachable {
            if isWWAN {
                let networkInfo = CTTelephonyNetworkInfo()
                let carrierType = networkInfo.serviceCurrentRadioAccessTechnology
                
                guard let carrierTypeName = carrierType?.first?.value else {
                    return "UNKNOWN"
                }
                
                switch carrierTypeName {
                case CTRadioAccessTechnologyGPRS, CTRadioAccessTechnologyEdge, CTRadioAccessTechnologyCDMA1x:
                    return "2G"
                case CTRadioAccessTechnologyLTE:
                    return "4G"
                default:
                    if #available(iOS 14.1, *) {
                        if carrierTypeName == CTRadioAccessTechnologyNR || carrierTypeName == CTRadioAccessTechnologyNRNSA {
                            return "5G"
                        }
                    }
                    return "3G"
                }
            } else {
                return "WIFI"
            }
        } else {
            return "NO INTERNET"
        }
    }
    
    public func getDeviceModel() -> String {
        var systemInfo = utsname()
        uname(&systemInfo)
        let machineMirror = Mirror(reflecting: systemInfo.machine)
        let identifier = machineMirror.children.reduce("") { identifier, element in
            guard let value = element.value as? Int8, value != 0 else { return identifier }
            return identifier + String(UnicodeScalar(UInt8(value)))
        }
        return identifier
    }
    
    public func getApplicationVisibilityState() -> String {
        switch UIApplication.shared.applicationState {
        case .active:
            return "foreground"
        case .background:
            return "background"
        case .inactive:
            return "inactive"
        @unknown default:
            return "unknown"
        }
    }
    
    public func _getMetricsJSON() -> String {
        let batteryInfo = getBatteryInfo()
        let cpuUsage = getCpuUsage()
        let memoryUsage = memoryUsage()
        let hardwareInfo = getHardwareInfo()
        
        // You may have to write functions for networkType and applicationState
        let networkType = getConnectionType()
        let applicationState = getApplicationVisibilityState()
        
        let metrics: [String: Any] = [
            "batteryPercentage": batteryInfo.batteryLevel,
            "isCharging": batteryInfo.isCharging,
            "ramUsageInMB": memoryUsage.used / 1048576,
            "networkType": networkType,
            "appState": applicationState
        ]
        
        guard let jsonData = try? JSONSerialization.data(withJSONObject: metrics, options: .prettyPrinted) else {
            return "{}"
        }
        
        return String(data: jsonData, encoding: .utf8) ?? "{}"
    }
    
    @objc public func getMetricsJSON() -> String {
        
        var metricsJSON = "{}"
        
        DispatchQueue.main.sync {
            let batteryInfo = getBatteryInfo()
            let cpuUsage = getCpuUsage()
            let memoryUsage = memoryUsage()
            let hardwareInfo = getHardwareInfo()
            
            // You may have to write functions for networkType and applicationState
            let networkType = getConnectionType()
            let applicationState = getApplicationVisibilityState()
            
            let metrics: [String: Any] = [
                "batteryPercentage": batteryInfo.batteryLevel,
                "isCharging": batteryInfo.isCharging,
                "ramUsageInMB": memoryUsage.used / 1048576,
                "networkType": networkType,
                "appState": applicationState
            ]
            
            if let jsonData = try? JSONSerialization.data(withJSONObject: metrics, options: .prettyPrinted),
               let jsonString = String(data: jsonData, encoding: .utf8) {
                metricsJSON = jsonString
            }
            
        }
        
        return metricsJSON
        
    }
    
    @objc public func getStaticDeviceMetrics() -> String {
        let deviceBrand = UIDevice.current.systemName
        let deviceModel = getDeviceModel()
        let systemVersion = UIDevice.current.systemVersion
        let deviceIdentifier = UIDevice.current.identifierForVendor?.uuidString ?? "N/A"
        var totalStorage: Int64 = 0
        var freeStorage: Int64 = 0
        let path = NSHomeDirectory()
        let networkType = getConnectionType()
        
        let fileManager = FileManager.default
        do {
            let attributes = try fileManager.attributesOfFileSystem(forPath: path)
            totalStorage = attributes[.systemSize] as? Int64 ?? 0
            freeStorage = attributes[.systemFreeSize] as? Int64 ?? 0
        }
        catch{
            totalStorage = -1
            freeStorage = -1
        }
        
        let device = UIDevice.current
        
        
        let memoryInfo = ProcessInfo.processInfo.physicalMemory
        let totalRamInMB = Double(memoryInfo) / 1024.0 / 1024.0
        
        let numCores = ProcessInfo.processInfo.processorCount
        let clockSpeed = UIDevice.current.getCPUSpeed()
        
        let staticMetrics: [String: Any] = [
            "deviceBrand": deviceBrand,
            "deviceModel": deviceModel,
            "systemSdkVersion": "iOS \(systemVersion)",
            "totalRamInMB": totalRamInMB,
            "numCores": numCores,
            "clockSpeedInHz": clockSpeed,
            "deviceIdentifier": deviceIdentifier,
            "totalInternalStorageInBytes": totalStorage,
            "freeInternalStorageInBytes": freeStorage,
            "networkType" : networkType
        ]
        
        do {
            let jsonData = try JSONSerialization.data(withJSONObject: staticMetrics)
            if let jsonString = String(data: jsonData, encoding: .utf8) {
                return jsonString
            }
        } catch {
            print("Error serializing static metrics to JSON: \(error.localizedDescription)")
        }
        
        return "{}"
    }
    
    
}

// https://stackoverflow.com/a/48018880
extension UIDevice {
    
    //Original Author: HAS
    // https://stackoverflow.com/questions/26028918/how-to-determine-the-current-iphone-device-model
    // Modified by Sam Trent
    
    /**********************************************
     *  getCPUName():
     *     Returns a hardcoded value of the current
     * devices CPU name.
     ***********************************************/
    func getCPUName() -> String {
        let processorNames = Array(CPUinfo().keys)
        return processorNames[0]
    }
    
    /**********************************************
     *  getCPUSpeed():
     *     Returns a hardcoded value of the current
     * devices CPU speed as specified by Apple.
     ***********************************************/
    func getCPUSpeed() -> String {
        let processorSpeed = Array(CPUinfo().values)
        return processorSpeed[0]
    }
    
    /**********************************************
     *  CPUinfo:
     *     Returns a dictionary of the name of the
     *  current devices processor and speed.
     ***********************************************/
    private func CPUinfo() -> Dictionary<String, String> {
        
#if targetEnvironment(simulator)
        let identifier = ProcessInfo().environment["SIMULATOR_MODEL_IDENTIFIER"]!
#else
        
        var systemInfo = utsname()
        uname(&systemInfo)
        let machineMirror = Mirror(reflecting: systemInfo.machine)
        let identifier = machineMirror.children.reduce("") { identifier, element in
            guard let value = element.value as? Int8 , value != 0 else
            { return identifier }
            return identifier + String(UnicodeScalar(UInt8(value)))
        }
#endif
        
        switch identifier {
            //        ipod
        case "iPod5,1":                                 return ["A5":"800 MHz"]
        case "iPod7,1":                                 return ["A8":"1.4 GHz"]
        case "iPod9,1":                                 return ["A10":"1.63 GHz"]
            //            iphone
        case "iPhone3,1", "iPhone3,2", "iPhone3,3":     return ["A4":"800 MHz"]
        case "iPhone4,1":                               return ["A5":"800 MHz"]
        case "iPhone5,1", "iPhone5,2":                  return ["A6":"1.3 GHz"]
        case "iPhone5,3", "iPhone5,4":                  return ["A6":"1.3 GHz"]
        case "iPhone6,1", "iPhone6,2":                  return ["A7":"1.3 GHz"]
        case "iPhone7,2":                               return ["A8":"1.4 GHz"]
        case "iPhone7,1":                               return ["A8":"1.4 GHz"]
        case "iPhone8,1":                               return ["A9":"1.85 GHz"]
        case "iPhone8,2":                               return ["A9":"1.85 GHz"]
        case "iPhone9,1", "iPhone9,3":                  return ["A10":"2.34 GHz"]
        case "iPhone9,2", "iPhone9,4":                  return ["A10":"2.34 GHz"]
        case "iPhone8,4":                               return ["A9":"1.85 GHz"]
        case "iPhone10,1", "iPhone10,4":                return ["A11":"2.39 GHz"]
        case "iPhone10,2", "iPhone10,5":                return ["A11":"2.39 GHz"]
        case "iPhone10,3", "iPhone10,6":                return ["A11":"2.39 GHz"]
        case "iPhone11,2", "iPhone11,4",
            "iPhone11,6",  "iPhone11,8":                return ["A12":"2.5 GHz"]
        case "iPhone12,1","iPhone12,3"
            ,"iPhone12,5", "iPhone12,8":                return ["A13":"2650 GHz"]
        case "iPhone13,4":                              return ["A14":"3.1 GHz"]
        case "iPhone14,5",
            "iPhone14,4",
            "iPhone14,2",
            "iPhone14,3", "iPhone14,6":                 return ["A15":"2x3.22 GHz"]
        case "iPhone15,1",
             "iPhone15,2",
             "iPhone15,3",
             "iPhone15,4",
             "iPhone15,5":                              return ["A16":"2x3.46 GHz"]

        case "iPhone16,1",
             "iPhone16,2",
             "iPhone16,3",
             "iPhone16,4",
             "iPhone16,5":                              return ["A17":"2x3.70 GHz"]

            //            ipad
        case "iPad2,1", "iPad2,2", "iPad2,3", "iPad2,4":return ["A5":"1.0 GHz"]
        case "iPad3,1", "iPad3,2", "iPad3,3":           return ["A5X":"1.0 GHz"]
        case "iPad3,4", "iPad3,5", "iPad3,6":           return ["A6X":"1.4 GHz"]
        case "iPad4,1", "iPad4,2", "iPad4,3":           return ["A7":"1.4 GHz"]
        case "iPad5,3", "iPad5,4":                      return ["A8X":"1.5 GHz"]
        case "iPad6,11", "iPad6,12":                    return ["A9":"1.85 GHz"]
        case "iPad2,5", "iPad2,6", "iPad2,7":           return ["A5":"1.0 GHz"]
        case "iPad4,4", "iPad4,5", "iPad4,6":           return ["A7":"1.3 GHz"]
        case "iPad4,7", "iPad4,8", "iPad4,9":           return ["A7":"1.3 GHz"]
        case "iPad5,1", "iPad5,2":                      return ["A8":"1.5 GHz"]
        case "iPad6,3", "iPad6,4":                      return ["A9X":"2.16 GHz"]
        case "iPad6,7", "iPad6,8":                      return ["A9X":"2.24 GHz"]
        case "iPad7,1", "iPad7,2",
            "iPad7,3", "iPad7,4":                       return ["A10X":"2.34 GHz"]
        case "iPad8,1", "iPad8,2",
            "iPad8,3", "iPad8,4":                       return ["A12X":"2.5 GHz"]
        case "iPad8,5", "iPad8,6",
            "iPad8,7", "iPad8,8",
            "iPad8,9", "iPad8,10",
            "iPad8,11", "iPad8,12":                     return ["A12Z":"2.5 GHz"]
        case "iPad13,4",
            "iPad13,5",
            "iPad13,6",
            "iPad13,7",
            "iPad13,8",
            "iPad13,9",
            "iPad13,10",
            "iPad13,11":                                return ["M1":"3.1 GHz"]
            
        default:                                        return ["N/A":"N/A"]
            
        }
    }
    
}

