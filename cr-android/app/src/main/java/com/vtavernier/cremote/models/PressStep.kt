package com.vtavernier.cremote.models

data class PressStep(var duration: StepDuration) : Step() {
    override fun toInt32(): Int {
        return ('P'.toInt() shl 24) or duration.toInt24()
    }

    override fun getFirstHeader(): String {
        return "Déclencher"
    }

    override fun getSubHeader(): String {
        return "Temps de pose : " + duration.toShutterSpeed()
    }
}