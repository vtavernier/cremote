package com.vtavernier.cremote.models

import com.google.gson.annotations.Expose
import java.util.*

data class LoopStep(@Expose(serialize = false) var targetStep: Step?, var count: Short) : Step() {
    var serializedTargetIndex: Int = 0

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
        return "Etape " + getTargetIndex(program) + ", " + count + " itération(s)"
    }

    override fun toString(): String {
        return getFirstHeader()
    }
}