package com.vtavernier.cremote.models

data class LoopStep(val targetIndex: Int) : Step() {
    override fun toInt32(): Int {
        return ('L'.toInt() shl 24) or targetIndex
    }
}