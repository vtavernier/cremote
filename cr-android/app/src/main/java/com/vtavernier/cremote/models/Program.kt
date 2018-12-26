package com.vtavernier.cremote.models

import com.google.gson.Gson
import com.google.gson.GsonBuilder
import com.google.gson.typeadapters.RuntimeTypeAdapterFactory

class Program {
    val steps = ArrayList<Step>()

    fun toBytes(): IntArray {
        val result = IntArray(steps.size) { 0 }
        steps.forEachIndexed { index, item ->
            result[index] = item.toInt32(this)
        }

        return result
    }

    fun toJson(): String {
        return getGson().toJson(this)
    }

    companion object {
        fun fromJson(str: String): Program {
            return getGson().fromJson(str, Program::class.java)
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