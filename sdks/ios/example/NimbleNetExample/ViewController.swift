/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import UIKit
import DeliteAI
import SwiftProtobuf

class ViewController: UIViewController {
    
    override func viewDidLoad() {
        super.viewDidLoad()
        let config = NimbleNetConfig(
            clientId: "testclient",
            clientSecret: BundleConfig.clientSecret,
            host: BundleConfig.host,
            deviceId: "hello-ios",
            debug: true,
            compatibilityTag: "proto-test",
            online: true
        )
        
        let res = NimbleNetApi.initialize(config: config)
        
        print("isInitialized? \(res)")
        let isReady = NimbleNetApi.isReady()
        print("isReady \(isReady)")
        while true {

            let modelStatus = NimbleNetApi.isReady()
            print(modelStatus)
            if modelStatus.status {
                break
            }
            Thread.sleep(forTimeInterval: 1)
        }
        
        var company = Generated_Company()
        company.companyID = "12345"
        company.companyName = "TechCorp"

        var department = Generated_Company.Department()
        department.departmentID = 1
        department.departmentName = "Engineering"

        var department2 = Generated_Company.Department()
        department2.departmentID = 2
        department2.departmentName = "management"
        
        print("department", department)
        print("2nd department", department2)

        
        var employee = Generated_Company.Department.Employee()
        employee.employeeID = "E001"
        employee.name = "John Doe"
        employee.title = "Software Engineer"

        var contactInfo = Generated_Company.Department.Employee.ContactInfo()
        contactInfo.phone = "+1234567890"
        
        employee.contactInfo = contactInfo
        var address = Generated_Address()
        address.street = "123 Main St"
        address.city = "San Francisco"
        address.state = "CA"
        address.zipCode = "94105"
        address.additionalInfo["landmark"] = "Near Central Park"
        address.additionalInfo["xyz"] = "abc"
        Google_Protobuf_Any.register(messageType: Generated_Address.self)
        do {
            contactInfo.address = try Google_Protobuf_Any(message: address)
            employee.contactInfo = contactInfo
            let messageType = Google_Protobuf_Any.messageType(forTypeURL: contactInfo.address.typeURL)!
            let address_decoded = try messageType.init(serializedBytes: contactInfo.address.value);
            print("Address decoded", address_decoded)
        } catch {
            print("Error in setting address")
        }
        
        var project = Generated_Company.Department.Employee.Project()
        project.projectID = "P001"
        project.projectName = "AI Research"
        project.role = "Lead Developer"

        employee.projects.append(project)
        department.employees.append(employee)
        company.departments.append(department)
        company.departments.append(department2)
        print("company in vc" ,company)
        print("----end")

       // print("--depts", depts)
                
        let modelInputs = [
            "inputData": NimbleNetTensor(
                data: company,
                datatype: DataType.FE_OBJ,
                shape: nil
            )
        ]

        let res2 = NimbleNetApi.runMethod(methodName: "test_as_is", inputs: modelInputs)
        print("result \(res2)")

    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
}

func delay(_ delay: Double, closure: @escaping () -> Void) {
    DispatchQueue.main.asyncAfter(deadline: .now() + delay) {
        closure()
    }
}
