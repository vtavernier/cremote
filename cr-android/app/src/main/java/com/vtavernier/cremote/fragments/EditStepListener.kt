package com.vtavernier.cremote.fragments

import com.vtavernier.cremote.models.Step

interface EditStepListener {
    fun getStep(): Step
    fun completeEdit()
    fun deleteEdit()
    fun canDelete(): Boolean
}