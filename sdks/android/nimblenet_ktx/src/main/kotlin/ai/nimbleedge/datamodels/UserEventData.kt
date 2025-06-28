package ai.nimbleedge.datamodels

data class UserEventData(var eventType: String? = null, var eventJsonString: String? = null) {
    override fun toString(): String {
        return "eventType: $eventType \n eventJsonString: $eventJsonString"
    }
}
