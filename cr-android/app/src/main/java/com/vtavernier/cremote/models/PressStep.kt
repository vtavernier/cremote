package com.vtavernier.cremote.models

import java.util.*

data class PressStep(var duration: StepDuration, var halfPressDuration: StepDuration) : Step() {
    override fun toInt32(program: Program): Collection<Int> {
        return arrayListOf(
                ('H'.toInt() shl 24) or halfPressDuration.toInt24(),
                ('P'.toInt() shl 24) or duration.toInt24()
        )
    }

    override fun getFirstHeader(): String {
        return "Déclencher (" + duration.toShutterSpeed() + ")"
    }

    override fun getSubHeader(program: Program): String {
        return "Délai : " + halfPressDuration.toShutterSpeed()
    }

    override fun toString(): String {
        return getFirstHeader()
    }
}