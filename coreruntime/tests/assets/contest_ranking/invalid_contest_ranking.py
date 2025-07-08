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
df =nm.Dataframe(contestJoinedClientTableSchema)
rawStore=nm.RawEventStore(tableName, "time", expiryTimeInMins)

@add_event(rawStore)
def enrichEvent(type,event):
    event["productid"] = "123"
    df.append(event)