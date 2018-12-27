package com.vtavernier.cremote.models

import java.util.*

data class LoopStep(val targetStep: Step, val count: Short) : Step() {
    fun getTargetIndex(program: Program): Int {
        return program.steps.indexOfFirst { it === targetStep }
    }

    override fun toInt32(program: Program): Collection<Int> {
        return Collections.singletonList(('L'.toInt() shl 24) or (count.toInt() shl 8) or (getTargetIndex(program)))
    }

    override fun getFirstHeader(): String {
        return "Recommencer"
    }

    override fun getSubHeader(program: Program): String {
        return "Etape " + getTargetIndex(program) + " " + count + " itération(s)"
    }
}