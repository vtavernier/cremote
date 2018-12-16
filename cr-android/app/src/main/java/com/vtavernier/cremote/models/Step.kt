package com.vtavernier.cremote.models

abstract class Step {
    abstract fun toInt32() : Int

    abstract fun getFirstHeader() : String
    abstract fun getSubHeader() : String
}