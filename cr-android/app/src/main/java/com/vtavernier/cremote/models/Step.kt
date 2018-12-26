package com.vtavernier.cremote.models

abstract class Step {
    abstract fun toInt32(program: Program): Int

    abstract fun getFirstHeader() : String
    abstract fun getSubHeader(program: Program): String
}