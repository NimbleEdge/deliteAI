# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import nimblenet as nm

tableName="ContestJoinedClient"

contestJoinedClientTableSchema = {
    "contestType": "string",
    "productid" : "int32",
    "roundid" : "int32",
    "winnerPercent" : "float",
    "prizeAmount": "double",
    "entryFee": "int32"
}

expiryTimeInMins = 60
rawStore = nm.RawEventStore(tableName,"time",expiryTimeInMins)
df=nm.Dataframe(contestJoinedClientTableSchema)

@add_event(rawStore)
def add_event_hook(type,event):
    df.append(event)

dataType="double"
contestTypeProcessor=df.processor(dataType).rollingWindow([18,15,10,8,6,1]).groupBy(["contestType"]).add_computation("contestType","Avg",0).create()
roundProductProcessor=df.processor(dataType).rollingWindow([21,17,9,8]).groupBy(["roundid","productid"]).add_computation("entryFee","Sum",0).create()
entryFeeProcessor=df.processor(dataType).rollingWindow([8,4,2]).groupBy(["entryFee"]).add_computation("entryFee","Count",0).create()
prizeAmountProcessor=df.processor(dataType).rollingWindow([16,7,3]).groupBy(["prizeAmount"]).add_computation("entryFee","Min",0).create()
winnerPercentProcessor=df.processor(dataType).rollingWindow([11,25,1]).groupBy(["winnerPercent"]).add_computation("prizeAmount","Max",0).create()
