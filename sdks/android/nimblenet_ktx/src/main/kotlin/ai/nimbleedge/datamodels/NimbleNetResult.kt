package ai.nimbleedge.datamodels

import ai.nimbleedge.impl.common.ERROR_CODES

class NimbleNetError(var code: Int = ERROR_CODES.KOTLIN_ERROR, var message: String = "") {
    private fun populateErrorObject(code: Int, message: String) {
        this.code = code
        this.message = message
    }

    override fun toString(): String {
        return "code: $code \n message: $message"
    }
}

class NimbleNetResult<T>(
    var status: Boolean = false,
    var payload: T?,
    var error: NimbleNetError? = NimbleNetError(),
) {
    override fun toString(): String {
        return "status: $status \nerror: $error \npayload: $payload"
    }
}
