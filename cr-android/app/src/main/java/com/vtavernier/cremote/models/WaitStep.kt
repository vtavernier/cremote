package com.vtavernier.cremote.models

import java.util.*

data class WaitStep(val duration: StepDuration) : Step() {
    override fun toInt32(program: Program): Collection<Int> {
        return Collections.singletonList(('W'.toInt() shl 24) or duration.toInt24())
    }

    override fun getFirstHeader(): String {
        return "Attendre " + duration.toUserString()
    }

    override fun getSubHeader(program: Program): String {
        return ""
    }
}