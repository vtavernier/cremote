package com.vtavernier.cremote.models

data class StepDuration(val type: DurationType, val amount: Short) {
    fun toInt24(bit7 : Byte = 0): Int {
        val typeId: Int = if (type == DurationType.Seconds) 1 else 0

        return ((((bit7.toInt() and 0x7F) shl 1) or typeId) shl 16) or amount.toInt()
    }
}