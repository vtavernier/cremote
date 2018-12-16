package com.vtavernier.cremote.models

data class StepDuration(var type: DurationType, var amount: Short) {
    fun toInt24(bit7 : Byte = 0): Int {
        val typeId: Int = type.value

        return ((((bit7.toInt() and 0x7F) shl 1) or typeId) shl 16) or amount.toInt()
    }

    fun toUserString(): String {
        return if (type == DurationType.Millis)
            amount.toString() + "ms"
        else
            amount.toString() + "s"
    }

    fun toShutterSpeed(): String {
        return if (type == DurationType.Millis)
            "1/" + (1000 / amount).toString() + "\""
        else
            amount.toString() + "\""
    }

    companion object {
        fun parse(str: String): StepDuration? {
            val r = Regex("^\\s*(1\\s*/)?\\s*(\\d+)\\s*\"\\s*$")
            val m = r.find(str)

            return if (m != null) {
                val v0 = m.groups[1]
                val v1 = m.groups[2]

                if (v0?.value != "") {
                    StepDuration(DurationType.Millis, (1000 / v1?.value!!.toInt()).toShort())
                } else {
                    StepDuration(DurationType.Seconds, v1?.value!!.toShort())
                }
            } else {
                null
            }
        }
    }
}