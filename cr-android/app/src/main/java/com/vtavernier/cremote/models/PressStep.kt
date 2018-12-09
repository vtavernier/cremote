package com.vtavernier.cremote.models

data class PressStep(val duration: StepDuration) : Step() {
    override fun toInt32(): Int {
        return ('P'.toInt() shl 24) or duration.toInt24()
    }
}