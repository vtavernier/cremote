package com.vtavernier.cremote.models

data class StepDuration(var type: DurationType, var amount: Short) {
    fun toInt24(bit6: Byte = 0): Int {
        val typeId: Int = type.value

        return ((((bit6.toInt() and 0x3F) shl 2) or typeId) shl 16) or amount.toInt()
    }

    fun toUserString(): String {
        return if (type == DurationType.Millis)
            amount.toString() + "ms"
        else if (type == DurationType.Seconds)
            amount.toString() + "s"
        else // if (type == DurationType.InverseSeconds)
            "1/" + amount.toString() + "\""
    }

    fun toShutterSpeed(): String {
        return if (type == DurationType.Millis) {
            if (amount < 1000)
                "1/" + (1000 / amount).toString() + "\""
            else
                (amount / 1000).toString() + if (amount % 1000 > 0) {
                    " 1/" + (1000 / (amount % 1000)).toString() + "\""
                } else {
                    "\""
                }
        } else if (type == DurationType.Seconds) {
            amount.toString() + "\""
        } else /* if (type == DurationType.InverseSeconds) */ {
            "1/" + amount.toString() + "\""
        }
    }

    companion object {
        fun parse(str: String): StepDuration? {
            val r = Regex("^\\s*(1\\s*/)?\\s*(\\d+)\\s*\"\\s*$")
            val m = r.find(str)

            return if (m != null) {
                val v0 = m.groups[1]
                val v1 = m.groups[2]

                if (v0?.value != "") {
                    StepDuration(DurationType.InverseSeconds, v1?.value!!.toShort())
                } else {
                    StepDuration(DurationType.Seconds, v1?.value!!.toShort())
                }
            } else {
                null
            }
        }
    }
}