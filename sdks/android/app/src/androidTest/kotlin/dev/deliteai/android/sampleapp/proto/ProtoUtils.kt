/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.android.sampleapp.proto

import dev.deliteai.datamodels.NimbleNetConfig
import dev.deliteai.impl.common.NIMBLENET_VARIANTS
import com.google.protobuf.Any
import com.google.protobuf.Message
import com.google.protobuf.util.JsonFormat
import dev.deliteai.nimblenet_ktx.BuildConfig
import generated.Address
import generated.Company
import generated.Company.Department
import generated.Company.Department.Employee
import generated.Company.Department.Employee.ContactInfo
import generated.Company.Department.Employee.Project
import generated.EmailAddress
import java.util.UUID

val config = NimbleNetConfig(
    clientId = BuildConfig.ANDROID_TEST_CLIENT_ID,
    host = BuildConfig.ANDROID_TEST_HOST,
    deviceId = "android-test",
    clientSecret = BuildConfig.ANDROID_TEST_CLIENT_SECRET,
    debug = true,
    initTimeOutInMs = 20000,
    compatibilityTag = "proto-test",
    libraryVariant = NIMBLENET_VARIANTS.STATIC,
    online = true
)

val typeRegistry = JsonFormat.TypeRegistry.newBuilder()
    .add(Company.getDescriptor())
    .add(Department.getDescriptor())
    .add(Employee.getDescriptor())
    .add(ContactInfo.getDescriptor())
    .add(Project.getDescriptor())
    .add(Address.getDescriptor())
    .add(EmailAddress.getDescriptor())
    .build()

data class ExpectedResult(
    val status: Boolean = true,
    val payload: Map<String, kotlin.Any> = emptyMap(),
    val error: String = ""
)

fun getProtoJson(message: Message) : String {
    return JsonFormat.printer()
        .usingTypeRegistry(typeRegistry)
        .includingDefaultValueFields()
        .print(message)
}

fun createCompany(
    numDepartments: Int,
    employeesPerDepartment: Int,
    projectsPerEmployee: Int
): Company {
    val companyBuilder = Company.newBuilder()
        .setCompanyId(UUID.randomUUID().toString())
        .setCompanyName("Tech Innovations")
        .setGlobal(true)
        .addAllOfficePincodes(listOf(10001, 20002, 30003))
    for (i in 0 until numDepartments) {
        val departmentBuilder = Department.newBuilder()
            .setDepartmentId(i.toLong())
            .setDepartmentName("Department $i")
            .setRevenue((1000*(i+1)).toFloat())
        for (j in 0 until employeesPerDepartment) {
            val employeeBuilder = Employee.newBuilder()
                .setEmployeeId("E$j")
                .setName("Employee $j")
            if (j % 2 == 0) {
                employeeBuilder.setTitle("Title $j")
            }
            val contactInfoBuilder = ContactInfo.newBuilder()
                .setPhone("555-000$j")
            if (i % 2 == 0) {
                val address = Address.newBuilder()
                    .setStreet("123 Street $j")
                    .setCity("City $j")
                    .setState("State $j")
                    .setZipCode("1000$j")
                    .putAdditionalInfo("landmark", "Near Park $j")
                    .putAdditionalInfo("building_code", "B-${i}0$j")
                    .putAdditionalInfo("apartment_number", "Apt ${j * 10}")
                    .addAllBuildings(listOf(1, 2, 3))
                    .build()

                contactInfoBuilder.setAddress(Any.pack(address))
            } else {
                val emailAddress = EmailAddress.newBuilder()
                    .setEmail("user$j@example.com")
                    .build()

                contactInfoBuilder.setAddress(Any.pack(emailAddress))
            }
            employeeBuilder.setContactInfo(contactInfoBuilder)
            // Add projects to each employee
            for (k in 0 until projectsPerEmployee) {
                employeeBuilder.addProjects(
                    Project.newBuilder()
                        .setProjectId("P$k")
                        .setProjectName("Project $k")
                        .setRole("Role $k")
                )
            }
            departmentBuilder.addEmployees(employeeBuilder)
        }
        companyBuilder.addDepartments(departmentBuilder)
    }
    return companyBuilder.build()
}

fun getGetterResponse(original: Company): Map<String, kotlin.Any> {
    val companyKeys = original.descriptorForType.fields.map { it.jsonName }
    val departmentNames = original.departmentsList.map { it.departmentName }
    val departmentIds = original.departmentsList.map { it.departmentId }
    val departmentRevenues = original.departmentsList.map { it.revenue }
    val dep0 = original.getDepartments(0)
    val employeeMap = dep0.employeesList.associate { employee ->
        employee.employeeId to mapOf("name" to employee.name,
            "title" to (if (employee.hasTitle()) employee.title else "absent"))
    }
    val address = dep0.getEmployees(0).contactInfo.address.unpack(Address::class.java)
    return mapOf(
        "response" to original,
        "companyId" to original.companyId,
        "companyIdPresent" to true,
        "global" to original.global,
        "companyKeys" to companyKeys,
        "departmentLen" to original.departmentsCount,
        "officePincodes" to original.officePincodesList,
        "departmentNames" to departmentNames,
        "departmentIds" to departmentIds,
        "departmentRevenues" to departmentRevenues,
        "employeeMap" to employeeMap,
        "employee0InfoMap" to address.additionalInfoMap,
        "employee0InfoLen" to address.additionalInfoCount,
        "landmarkInInfoMap" to true,
        "employee0Landmark" to (address.additionalInfoMap["landmark"] as String),
        "employee0InfoKeys" to address.additionalInfoMap.keys.toList()
    )
}

fun getUpdatedCompany1(original: Company): Company {
    val updatedCompany = original.toBuilder()
    updatedCompany.companyId = "updatedId"
    updatedCompany.global = false
    val dep0 = updatedCompany.getDepartments(0).toBuilder()
    dep0.departmentName = "updatedName"
    dep0.departmentId = 1111
    dep0.revenue = 111.5F
    updatedCompany.setDepartments(0, dep0.build())
    return updatedCompany.build()
}

fun getUpdatedCompany2(original: Company): Company {
    val updatedCompany = original.toBuilder()
    val dep0 = updatedCompany.getDepartments(0).toBuilder()
    val emp0 = dep0.getEmployees(0).toBuilder()
    val address = emp0.contactInfo.address.unpack(Address::class.java).toBuilder()
    address.putAdditionalInfo("landmark", "updatedLandmark")
        .putAdditionalInfo("newKey", "newValue")
    val updatedAddress = Any.pack(address.build())
    emp0.contactInfo = emp0.contactInfo.toBuilder().setAddress(updatedAddress).build()
    dep0.setEmployees(0, emp0.build())
    updatedCompany.setDepartments(0, dep0.build())
    return updatedCompany.build()
}

fun getUpdatedCompany3(original: Company): Company {
    val updatedCompany = original.toBuilder()
    val dep0 = updatedCompany.getDepartments(0).toBuilder()
    dep0.setEmployees(0, dep0.getEmployees(1))
    updatedCompany.setDepartments(0, dep0.build())
    return updatedCompany.build()
}

fun getUpdatedCompany4(original: Company): Company {
    val updatedCompany = original.toBuilder()
    val dep0 = updatedCompany.getDepartments(0).toBuilder()
    val emp0 = dep0.getEmployees(0)
    dep0.setEmployees(0, dep0.getEmployees(1))
    dep0.setEmployees(1, dep0.getEmployees(2))
    dep0.setEmployees(2, emp0)
    updatedCompany.setDepartments(0, dep0.build())
    return updatedCompany.build()
}

fun getUpdatedCompany5(original: Company): Company {
    val updatedCompany = original.toBuilder()
    val update = updatedCompany.getDepartments(1).getEmployees(0).contactInfo.address
    val dep0 = updatedCompany.getDepartments(0).toBuilder()
    val emp0 = dep0.getEmployees(0).toBuilder()
    val c0 = emp0.contactInfo.toBuilder().setAddress(update).build()
    emp0.setContactInfo(c0)
    dep0.setEmployees(0, emp0.build())
    updatedCompany.setDepartments(0, dep0.build())
    return updatedCompany.build()
}

fun getUpdatedCompany6(original: Company): Company {
    val updatedCompany = original.toBuilder()
    val updatedPincodes = updatedCompany.officePincodesList.map { it + 1 }
    updatedCompany.clearOfficePincodes().addAllOfficePincodes(updatedPincodes)
    return updatedCompany.build()
}

fun getUpdatedCompany7(original: Company): Company {
    return original.toBuilder().clearDepartments().build()
}

fun getUpdatedCompany8(original: Company): Company {
    val updatedCompany = original.toBuilder()
    val dep0 = updatedCompany.getDepartments(0).toBuilder()
    val emp0 = dep0.getEmployees(0).toBuilder()
    val address = emp0.contactInfo.address.unpack(Address::class.java).toBuilder()
    address.clearAdditionalInfo()
    val updatedAddress = Any.pack(address.build())
    emp0.contactInfo = emp0.contactInfo.toBuilder().setAddress(updatedAddress).build()
    dep0.setEmployees(0, emp0.build())
    updatedCompany.setDepartments(0, dep0.build())
    return updatedCompany.build()
}

fun getUpdatedCompany9(original: Company): Company {
    val updatedCompany = original.toBuilder()
    updatedCompany.clearCompanyName()
    val dep0 = updatedCompany.getDepartments(0).toBuilder()
    dep0.clearRevenue()
    val emp0 = dep0.getEmployees(0).toBuilder()
    emp0.clearContactInfo()
    dep0.setEmployees(0, emp0.build())
    updatedCompany.setDepartments(0, dep0.build())
    return updatedCompany.build()
}

fun getUpdatedCompany10(original: Company): Company {
    val updatedCompany = original.toBuilder()

    val mutablePincodes = updatedCompany.officePincodesList.toMutableList()
    mutablePincodes.removeAt(1)
    updatedCompany.clearOfficePincodes().addAllOfficePincodes(mutablePincodes)

    val departmentsBuilder = updatedCompany.getDepartmentsBuilder(0)
    departmentsBuilder.removeEmployees(0)

    val employeeBuilder = departmentsBuilder.getEmployeesBuilder(1)
    val contactInfoBuilder = employeeBuilder.contactInfoBuilder
    val addressAny = contactInfoBuilder.address
    val addressBuilder = Address.newBuilder()
    if (addressAny.`is`(Address::class.java)) {
        addressAny.unpack(Address::class.java).let { unpackAddress -> addressBuilder.mergeFrom(unpackAddress)}
    } else {
        return updatedCompany.build()
    }
    val mutableMap = addressBuilder.additionalInfoMap.toMutableMap()
    mutableMap.remove("landmark")
    addressBuilder.clearAdditionalInfo().putAllAdditionalInfo(mutableMap)

    val modifiedAddressAny = Any.pack(addressBuilder.build())
    contactInfoBuilder.address = modifiedAddressAny
    return updatedCompany.build()
}

fun getUpdatedCompany11(original: Company): Company {
   val updatedCompany = original.toBuilder()

   updatedCompany.addOfficePincodes(11111)

   updatedCompany.addDepartments(original.getDepartments(1))
   return updatedCompany.build()
}
