package com.vtavernier.cremote.fragments

import android.content.Context
import android.support.v4.app.FragmentActivity
import android.view.LayoutInflater
import android.view.View
import android.widget.ArrayAdapter
import android.widget.Spinner
import com.vtavernier.cremote.R
import com.vtavernier.cremote.models.LoopStep

class EditLoopFragment : BaseEditFragment<LoopStep>() {
    override fun initView(it: FragmentActivity, step: LoopStep): View {
        val view = (it.getSystemService(Context.LAYOUT_INFLATER_SERVICE) as LayoutInflater)
                .inflate(R.layout.edit_loop_fragment, null)


        return view
    }

    override fun updateStep(view: View, step: LoopStep) {

    }
}