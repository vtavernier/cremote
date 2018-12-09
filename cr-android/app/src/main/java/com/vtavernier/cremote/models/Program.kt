package com.vtavernier.cremote.models

class Program {
    val steps = ArrayList<Step>()

    fun toBytes(): IntArray {
        val result = IntArray(steps.size) { 0 }
        steps.forEachIndexed { index, item ->
            result[index] = item.toInt32()
        }

        return result
    }
}