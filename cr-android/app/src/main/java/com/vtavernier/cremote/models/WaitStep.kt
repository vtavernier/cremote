package com.vtavernier.cremote.models

data class WaitStep(val duration: StepDuration) : Step() {
    override fun toInt32(): Int {
        return ('W'.toInt() shl 24) or duration.toInt24()
    }

    override fun getFirstHeader(): String {
        return "Attendre " + duration.toUserString()
    }

    override fun getSubHeader(): String {
        return ""
    }
}