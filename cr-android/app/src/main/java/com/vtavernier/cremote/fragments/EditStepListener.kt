package com.vtavernier.cremote.fragments

import com.vtavernier.cremote.models.Program
import com.vtavernier.cremote.models.Step

interface EditStepListener {
    fun getStep(): Step
    fun getProgram(): Program
    fun getInsertLocation(): Int
    fun completeEdit()
    fun deleteEdit()
    fun canDelete(): Boolean
}