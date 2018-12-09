package com.vtavernier.cremote.models

data class LoopStep(val targetIndex: Byte, val count: Short) : Step() {
    override fun toInt32(): Int {
        return ('L'.toInt() shl 24) or (count.toInt() shl 8) or (targetIndex.toInt())
    }
}