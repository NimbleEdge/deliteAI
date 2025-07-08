/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import XCTest
import DeliteAI
import Foundation
import SwiftProtobuf

class ProtoTest: XCTestCase {
    
    override func setUp() {
        super.setUp()
 
        let config = NimbleNetConfig(
            clientId: "testclient",
            clientSecret: BundleConfig.clientSecret,
            host: BundleConfig.host,
            deviceId: "ios-proto-test",
            debug: true,
            compatibilityTag: "proto-test",
            online: true
        )
        
        let res = NimbleNetApi.initialize(config: config)
        XCTAssertTrue(res.status, "Initialization failed")
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
    }
    
    
    func testProto() {
        do {
            let company = try createSampleCompany(numDepartments: 2, employeesPerDepartment: 3)
            
            let testCases: [(methodName: String, expectedResult: ExpectedResult)] = [
                ("test_as_is", ExpectedResult(payload: company)),
                ("test_valid_getters", ExpectedResult(payload: company)),
                ("test_invalid_key", ExpectedResult(status: false, error: "Field invalidKey not recognised for proto")),
                ("test_invalid_key_in_map", ExpectedResult(status: false, error: "Key invalidKey not found in map")),
                ("test_invalid_index", ExpectedResult(status: false, error: "index out of bound")),
                ("test_setters_object", ExpectedResult(payload: getUpdatedCompany1(original: company))),
                ("test_setters_map", ExpectedResult(payload: try getUpdatedCompany2(original: company))),
                ("test_setters_list", ExpectedResult(payload: getUpdatedCompany3(original: company))),
                ("test_arrange_list", ExpectedResult(payload: getUpdatedCompany4(original: company))),
                ("test_any_object", ExpectedResult(payload: getUpdatedCompany5(original: company))),
                ("test_setters_list_each_index", ExpectedResult(payload: getUpdatedCompany6(original: company))),
                ("test_set_invalid_key", ExpectedResult(status: false, error: "Field invalidKey not recognised for proto")),
                ("test_set_invalid_index", ExpectedResult(status: false, error: "index out of bound")),
                ("test_invalid_type_in_object", ExpectedResult(status: false, error: "error decoding proto")),
                ("test_invalid_type_in_list", ExpectedResult(status: false, error: "error decoding proto")),
                ("test_invalid_type_in_map", ExpectedResult(status: false, error: "error decoding proto")),
                ("test_arrange_list_empty", ExpectedResult(payload: getUpdatedCompany7(original: company))),
            ]
            
            for testCase in testCases {
                runMethodTest(company: company, methodName: testCase.methodName, expectedResult: testCase.expectedResult)
            }
            
        } catch {
            XCTFail("Failed to run test cases: \(error.localizedDescription)")
        }
    }
    
    private func runMethodTest(company: Generated_Company, methodName: String, expectedResult: ExpectedResult) {
        let modelInputs = [
            "inputData": NimbleNetTensor(
                data: company,
                datatype: DataType.FE_OBJ,
                shape: nil
            )
        ]
        
        let res = NimbleNetApi.runMethod(methodName: methodName, inputs: modelInputs)
        
        XCTAssertEqual(res.status, expectedResult.status, "[\(methodName)] Status mismatch")
        
        if(res.status){
            let updatedCompany = res.payload!["response"]!.data as! Generated_Company
            XCTAssertEqual(expectedResult.payload, updatedCompany, "[\(methodName)] Payload does not match")
        }
        else{
            let errorMessage: String = res.error!.message
            print(errorMessage)
            XCTAssertTrue(errorMessage.contains(expectedResult.error), "[\(methodName)] Error message does not match")
        }
    }
}

struct ExpectedResult {
    var status: Bool = true
    var payload: Generated_Company? = nil
    var error: String = ""
}

func createSampleCompany(numDepartments: Int, employeesPerDepartment: Int) throws -> Generated_Company {
    var company = Generated_Company()
    company.companyID = "12345"
    company.companyName = "TechCorp"
    company.global = true
    company.officePincodes = [10001, 20002, 30003]
    
    for i in 0..<numDepartments {
        var department = Generated_Company.Department()
        department.departmentID = Int64(i+100)
        department.departmentName = "Department \(i)"
        department.revenue = Float(1000*(i+1))
        
        for j in 0..<employeesPerDepartment {
            var employee = Generated_Company.Department.Employee()
            employee.employeeID = "E\(j)"
            employee.name = "Employee \(j)"
            if j%2 == 0 {
                employee.title = "Title \(j)"
            }
            
            var contactInfo = Generated_Company.Department.Employee.ContactInfo()
            contactInfo.phone = "+12345\(j)"
            
            if i % 2 == 0 {
                var address = Generated_Address()
                address.street = "Street \(j)"
                address.city = "City \(j)"
                address.state = "State \(j)"
                address.zipCode = "1000\(j)"
                address.additionalInfo["landmark"] = "Near Park \(j)"
                address.buildings = [1, 2, 3]
                
                Google_Protobuf_Any.register(messageType: Generated_Address.self)
                contactInfo.address = try Google_Protobuf_Any(message: address)
            } else {
                var emailAddress = Generated_EmailAddress()
                emailAddress.email = "user\(j)@example.com"
                
                Google_Protobuf_Any.register(messageType: Generated_EmailAddress.self)
                contactInfo.address = try Google_Protobuf_Any(message: emailAddress)
            }
            
            employee.contactInfo = contactInfo
            
            var project = Generated_Company.Department.Employee.Project()
            project.projectID = "P\(j)"
            project.projectName = "Project \(j)"
            project.role = "Role \(j)"
            employee.projects.append(project)
            
            department.employees.append(employee)
        }

        company.departments.append(department)
    }
    
    return company
}

func getUpdatedCompany1(original: Generated_Company) -> Generated_Company {
    var updatedCompany = original
    updatedCompany.companyID = "updatedId"
    updatedCompany.global = false
    var dep0 = updatedCompany.departments[0]
    dep0.departmentName = "updatedName"
    dep0.departmentID = 1111
    dep0.revenue = 111.5
    updatedCompany.departments[0] = dep0
    return updatedCompany
}

func getUpdatedCompany2(original: Generated_Company) throws -> Generated_Company {
    var updatedCompany = original
    var dep0 = updatedCompany.departments[0]
    var emp0 = dep0.employees[0]
    
    let addressAny = emp0.contactInfo.address
    var updatedAddress = try Generated_Address(serializedData: addressAny.value)
    updatedAddress.additionalInfo["landmark"] = "updatedLandmark"
    updatedAddress.additionalInfo["newKey"] = "newValue"
    emp0.contactInfo.address = try Google_Protobuf_Any(message: updatedAddress)
    
    dep0.employees[0] = emp0
    updatedCompany.departments[0] = dep0
    return updatedCompany
}

func getUpdatedCompany3(original: Generated_Company) -> Generated_Company {
    var updatedCompany = original
    var dep0 = updatedCompany.departments[0]
    dep0.employees[0] = dep0.employees[1]
    updatedCompany.departments[0] = dep0
    return updatedCompany
}

func getUpdatedCompany4(original: Generated_Company) -> Generated_Company {
    var updatedCompany = original
    var dep0 = updatedCompany.departments[0]
    let emp0 = dep0.employees[0]
    dep0.employees[0] = dep0.employees[1]
    dep0.employees[1] = dep0.employees[2]
    dep0.employees[2] = emp0
    updatedCompany.departments[0] = dep0
    return updatedCompany
}

func getUpdatedCompany5(original: Generated_Company) -> Generated_Company {
    var updatedCompany = original
    let update = updatedCompany.departments[1].employees[0].contactInfo.address
    var dep0 = updatedCompany.departments[0]
    var emp0 = dep0.employees[0]
    var c0 = emp0.contactInfo
    c0.address = update
    emp0.contactInfo = c0
    dep0.employees[0] = emp0
    updatedCompany.departments[0] = dep0
    return updatedCompany
}

func getUpdatedCompany6(original: Generated_Company) -> Generated_Company {
    var updatedCompany = original
    updatedCompany.officePincodes = updatedCompany.officePincodes.map { $0 + 1 }
    return updatedCompany
}

func getUpdatedCompany7(original: Generated_Company) -> Generated_Company {
    var updatedCompany = original
    updatedCompany.departments.removeAll()
    return updatedCompany
}
