package com.vtavernier.cremote.models

import com.google.gson.Gson
import com.google.gson.GsonBuilder
import com.google.gson.typeadapters.RuntimeTypeAdapterFactory

class Program {
    val steps = ArrayList<Step>()

    fun toBytes(): ByteArray {
        val ints = arrayListOf<Int>()
        steps.forEach { item ->
            ints.addAll(item.toInt32(this))
        }

        val bytes = ByteArray(ints.size * 4)
        ints.forEachIndexed { index, item ->
            bytes[index * 4 + 0] = ((item and 0xFF000000.toInt()) shr 24).toByte()
            bytes[index * 4 + 1] = ((item and 0x00FF0000.toInt()) shr 16).toByte()
            bytes[index * 4 + 2] = ((item and 0x0000FF00.toInt()) shr 8).toByte()
            bytes[index * 4 + 3] = ((item and 0x000000FF.toInt())).toByte()
        }

        return bytes
    }

    fun toJson(): String {
        for (step in steps) {
            if (step is LoopStep) {
                step.serializedTargetIndex = step.getTargetIndex(this)
            }
        }

        return getGson().toJson(this)
    }

    companion object {
        fun fromJson(str: String): Program {
            val program = getGson().fromJson(str, Program::class.java)

            for (step in program.steps) {
                if (step is LoopStep) {
                    step.targetStep = program.steps[step.serializedTargetIndex]
                }
            }

            return program
        }

        private fun getGson(): Gson {
            val runtimeTypeAdapterFactory =
                    RuntimeTypeAdapterFactory.of(Step::class.java, "type")
                            .registerSubtype(LoopStep::class.java, "loop")
                            .registerSubtype(PressStep::class.java, "press")
                            .registerSubtype(WaitStep::class.java, "wait")

            return GsonBuilder()
                    .registerTypeAdapterFactory(runtimeTypeAdapterFactory)
                    .create()
        }
    }
}
