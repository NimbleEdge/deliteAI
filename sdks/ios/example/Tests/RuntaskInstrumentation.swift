/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import XCTest
import DeliteAI
import Foundation

class runMethodInstrumentation: XCTestCase {
    
    var isModelReady = false
    
    override func setUp() {
        super.setUp()
        
        let context = UIApplication.shared.delegate as! UIApplicationDelegate
 
        let config = NimbleNetConfig(clientId: "testclient", clientSecret: BundleConfig.clientSecret, host: BundleConfig.host, deviceId: "nimon", debug: true, compatibilityTag: "android-output-verification", online: true)
        
        let res = NimbleNetApi.initialize(config: config)
        sleep(2)
    }
    
    
    func testInt32Conversion() {
        var timeElapsed = 0;
        while true {
            if(timeElapsed > 15){
                XCTFail("time limit exceeded for init")
                return
            }
            
            let modelStatus = NimbleNetApi.isReady()
            print(modelStatus)
            if(modelStatus.status){
                break
            }
            Thread.sleep(forTimeInterval: 1)
            timeElapsed+=1
        }
        
        let dataArray:[Int32] = [1,2,3,4]
        
        let modelInputs = [
            "model_input":NimbleNetTensor(
                data: dataArray,
                datatype: DataType.int32,
                shape: [4]
            )
        ]
        
        let res = NimbleNetApi.runMethod(methodName: "int32", inputs: modelInputs)
        
        if(res.status){
            var data = res.payload!
            XCTAssertTrue( (data["model_output"]?.data as? [Int32]) == dataArray.map { $0 * 2 }, "Output data is not as expected")
        }
        else{
            XCTFail("Predict Failed")
        }
    }
    
    func testVerifySingularInt64Conversion() {
        var timeElapsed = 0
        while true {
            if timeElapsed > 15 {
                XCTFail("time limit exceeded for init")
                return
            }

            let modelStatus = NimbleNetApi.isReady()
            print(modelStatus)
            if modelStatus.status {
                break
            }
            Thread.sleep(forTimeInterval: 1)
            timeElapsed += 1
        }
        let singularData:Int64 = 69
            let modelInput = NimbleNetTensor(data: singularData, datatype: .int64, shape: nil)
        let modelInputs: [String: NimbleNetTensor] = ["model_input": modelInput]

        let result = NimbleNetApi.runMethod(methodName: "int64_singular", inputs: modelInputs)

        XCTAssertTrue(result.status, "Predict Failed")

        if let outputs = result.payload {
            guard let output = outputs["model_output"] else {
                XCTFail("Output not found")
                return
            }
            XCTAssertEqual(output.data as! Int64, singularData * 2)
            XCTAssertEqual(output.type, .int64)
            XCTAssertEqual(output.shape, [])
            XCTAssertEqual(outputs.numOutputs, 1)
        } else {
            XCTFail("No outputs in the result")
        }
    }
    
    func testInt64Conversion() {
        var timeElapsed = 0;
        while true {
            if(timeElapsed > 15){
                XCTFail("time limit exceeded for init")
                return
            }
            
            let modelStatus = NimbleNetApi.isReady()
            print(modelStatus)
            if(modelStatus.status){
                break
            }
            Thread.sleep(forTimeInterval: 1)
            timeElapsed+=1
        }
        
        let dataArray:[Int64] = [1,2,3,4]
        
        let modelInputs = [
            "model_input":NimbleNetTensor(
                data: dataArray,
                datatype: DataType.int64,
                shape: [4]
            )
        ]
        
        let res = NimbleNetApi.runMethod(methodName: "int64", inputs: modelInputs)
        
        if(res.status){
            var data = res.payload!
            XCTAssertTrue(data["model_output"]?.data as! [Int64] == dataArray.map { $0 * 2 }, "Output data is not as expected")
        }
        else{
            XCTFail("Predict Failed")
        }
    }
    
    func testFloat32Conversion() {
        var timeElapsed = 0;
        while true {
            if(timeElapsed > 15){
                XCTFail("time limit exceeded for init")
                return
            }
            
            let modelStatus = NimbleNetApi.isReady()
            print(modelStatus)
            if(modelStatus.status){
                break
            }
            Thread.sleep(forTimeInterval: 1)
            timeElapsed+=1
        }
        
        let dataArray:[Float32] = [1.0,2.0,3.0,4.0]
        
        let modelInputs = [
            "model_input":NimbleNetTensor(
                data: dataArray,
                datatype: DataType.float,
                shape: [4]
            )
        ]
        
        let res = NimbleNetApi.runMethod(methodName: "float32", inputs: modelInputs)
        
        if(res.status){
            var data = res.payload!
            XCTAssertTrue(data["model_output"]?.data as! [Float32] == dataArray.map { $0 * 2 }, "Output data is not as expected")
        }
        else{
            XCTFail("Predict Failed")
        }
    }
    
    func testFloat64Conversion() {
        var timeElapsed = 0;
        while true {
            if(timeElapsed > 15){
                XCTFail("time limit exceeded for init")
                return
            }
            
            let modelStatus = NimbleNetApi.isReady()
            print(modelStatus)
            if(modelStatus.status){
                break
            }
            Thread.sleep(forTimeInterval: 1)
            timeElapsed+=1
        }
        
        let dataArray:[Float64] = [1.0,2.0,3.0,4.0]
        
        let modelInputs = [
            "model_input":NimbleNetTensor(
                data: dataArray,
                datatype: DataType.double,
                shape: [4]
            )
        ]
        
        let res = NimbleNetApi.runMethod(methodName: "float64", inputs: modelInputs)
        
        if(res.status){
            var data = res.payload!
            XCTAssertTrue(data["model_output"]?.data as! [Float64] == dataArray.map { $0 * 2 }, "Output data is not as expected")
        }
        else{
            XCTFail("Predict Failed")
        }
    }
    
    
    func testVerifySingularInt32Conversion(){
        var timeElapsed = 0
        while true {
            if timeElapsed > 15 {
                XCTFail("time limit exceeded for init")
                return
            }
            
            let modelStatus = NimbleNetApi.isReady()
            print(modelStatus)
            if modelStatus.status {
                break
            }
            Thread.sleep(forTimeInterval: 1)
            timeElapsed += 1
        }
        
        let singularData:Int32 = 69
            let modelInput = NimbleNetTensor(data: singularData, datatype: .int32, shape: nil)
    
        let modelInputs: [String: NimbleNetTensor] = ["model_input": modelInput]
        let result = NimbleNetApi.runMethod(methodName: "int32_singular", inputs: modelInputs)
        
        XCTAssertTrue(result.status, "Predict Failed")
        
        if let outputs = result.payload {
            guard let output = outputs["model_output"] else {
                XCTFail("Output not found")
                return
            }
            XCTAssertEqual(output.data as! Int32, (singularData) * 2)
            XCTAssertEqual(output.shape, [])
            XCTAssertEqual(outputs.numOutputs, 1)
        } else {
            XCTFail("No outputs in the result")
        }
    }
    
    func testVerifySingularFP32Conversion() {
        var timeElapsed = 0
        while true {
            if timeElapsed > 15 {
                XCTFail("time limit exceeded for init")
                return
            }
            
            let modelStatus = NimbleNetApi.isReady()
            print(modelStatus)
            if modelStatus.status {
                break
            }
            Thread.sleep(forTimeInterval: 1)
            timeElapsed += 1
        }
        let singulardata:Float = 69
            let modelInput = NimbleNetTensor(data: singulardata, datatype: .float, shape: nil)
        let modelInputs: [String: NimbleNetTensor] = ["model_input": modelInput]
        
        let result = NimbleNetApi.runMethod(methodName: "float32_singular", inputs: modelInputs)
        
        XCTAssertTrue(result.status, "Predict Failed")
        
        if let outputs = result.payload {
            guard let output = outputs["model_output"] else {
                XCTFail("Output not found")
                return
            }
            
            XCTAssertEqual(output.data as! Float, singulardata * 2)
            XCTAssertEqual(output.type, .float)
            XCTAssertEqual(output.shape, [])
            XCTAssertEqual(outputs.numOutputs, 1)
        } else {
            XCTFail("No outputs in the result")
        }
    }

    func testVerifySingularFP64Conversion() {
        var timeElapsed = 0
        while true {
            if timeElapsed > 15 {
                XCTFail("time limit exceeded for init")
                return
            }
            
            let modelStatus = NimbleNetApi.isReady()
            print(modelStatus)
            if modelStatus.status {
                break
            }
            Thread.sleep(forTimeInterval: 1)
            timeElapsed += 1
        }
        let singularData: Double = 69.0
            let modelInput = NimbleNetTensor(data: Double(69.0), datatype: .double, shape: nil)
        let modelInputs: [String: NimbleNetTensor] = ["model_input": modelInput]
        let result = NimbleNetApi.runMethod(methodName: "float64_singular", inputs: modelInputs)
    
        XCTAssertTrue(result.status, "Predict Failed")
        
        if let outputs = result.payload {
            guard let output = outputs["model_output"] else {
                XCTFail("Output not found")
                return
            }
            
            XCTAssertEqual(output.data as! Double, singularData * 2)
            XCTAssertEqual(output.type, .double)
            XCTAssertEqual(output.shape, [])
            XCTAssertEqual(outputs.numOutputs, 1)
        } else {
            XCTFail("No outputs in the result")
        }
    }
    
    func testNullHandlingJSONArray() throws {
        let jsonArray: [Any?] = [
            [
                "string": "value",
                "number": 123,
                "boolean": true,
                "nullValue": nil,
                "array": ["one", 2, true],
                "nestedObject": [
                    "key1": "value1",
                    "key2": 99
                ]
            ],
            "hello",
            "world",
            12.47,
            -87,
            nil,
            234234234,
            ["some", nil]
        ]
        
        let modelInputs: [String: NimbleNetTensor] = [
            "input": NimbleNetTensor(data: jsonArray, datatype: .jsonArray, shape: [8])
        ]

        let result = NimbleNetApi.runMethod(methodName: "test_null_jsonarray", inputs: modelInputs)

        guard let payload = result.payload,
              let modelOutput = payload["key"]?.data as? String else {
            XCTFail("Failed to retrieve the model output.")
            return
        }

        XCTAssertTrue(result.status && modelOutput == "NO")
    }
    
    func testNullHandlingJSON() throws {

        let jsonObject: [String: Any?] = [
            "string": "value",
            "number": 123,
            "boolean": true,
            "nullValue": nil,
            "array": ["one", 2, true],
            "nestedObject": [
                "key1": "value1",
                "key2": 99
            ]
        ]

        let modelInputs: [String: NimbleNetTensor] = [
            "input": NimbleNetTensor(data: jsonObject, datatype: .json, shape: nil)
        ]

        let result =  NimbleNetApi.runMethod(methodName: "test_null_json", inputs: modelInputs)
        if(!result.status){
            XCTFail("not ready")
        }
        guard let payload = result.payload,
              let modelOutput = payload["key"]?.data as? String else {
            XCTFail("Failed to retrieve the model output.")
            return
        }
        XCTAssertTrue(result.status && modelOutput == "NO")
   
    }
    
}
