/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 

import Foundation

@objc public class ProtoListWrapper: NSObject {
    var protoList: [AnyObject]
    
    init (array: [AnyObject]) {
        self.protoList = array
    }
    
    
    @objc public func get_value(index: Int, value: UnsafeMutablePointer<AnyObject> ) -> UnsafeMutablePointer<NimbleNetStatus>? {
        if(index < 0 || index >= self.protoList.count){
            return createNimbleNetStatus("index out of bound")
        }
        value.pointee = protoList[index]
        return nil
    }
    
    @objc public func set_value(index: Int, value: AnyObject) -> UnsafeMutablePointer<NimbleNetStatus>? {
        if(index < 0 || index >= self.protoList.count){
            return createNimbleNetStatus("index out of bound")
        }
        protoList[index] = value
        return nil
    }
    
    @objc public func count() -> Int {
        return protoList.count
    }
    
    @objc public func rearrange(indices:UnsafePointer<Int32>, numIndices: Int) -> ProtoListWrapper {
        var newList: [AnyObject] = []
        for i in 0..<numIndices {
            newList.append(protoList[Int(indices.advanced(by: i).pointee)])
        }
        return ProtoListWrapper(array: newList)
    }
    
    @objc public func ios_object_to_string(str: UnsafeMutablePointer<UnsafeMutablePointer<CChar>>) -> UnsafeMutablePointer<NimbleNetStatus>? {
        let objectString = "\(self.protoList)"
        objectString.withCString { swiftString in
            str.pointee = strdup(swiftString)
        }
        return nil
    }
}
