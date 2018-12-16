package com.vtavernier.cremote.models

enum class DurationType(val value: Int) {
    Millis(0),
    Seconds(1);

    companion object {
        fun fromInt(i: Int?): DurationType? {
            return if (i == 0)
                Millis
            else if (i == 1)
                Seconds
            else
                null
        }
    }
}