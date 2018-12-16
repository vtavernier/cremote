package com.vtavernier.cremote.fragments

import android.content.Context
import android.support.v4.app.FragmentActivity
import android.view.LayoutInflater
import android.view.View
import android.widget.ArrayAdapter
import android.widget.EditText
import android.widget.Spinner
import com.vtavernier.cremote.R
import com.vtavernier.cremote.models.DurationType
import com.vtavernier.cremote.models.WaitStep

class EditWaitFragment : BaseEditFragment<WaitStep>() {
    override fun initView(it: FragmentActivity, step: WaitStep): View {
        val view = (it.getSystemService(Context.LAYOUT_INFLATER_SERVICE) as LayoutInflater)
                .inflate(R.layout.edit_wait_fragment, null)

        val typeSpinner = view.findViewById<Spinner>(R.id.duration_type)
        ArrayAdapter.createFromResource(
                it,
                R.array.duration_type_names,
                android.R.layout.simple_spinner_item
        ).also { adapter ->
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
            typeSpinner.adapter = adapter
        }

        typeSpinner.setSelection(step.duration.type.value)

        val durationText = view.findViewById<EditText>(R.id.duration_count)
        durationText.setText(step.duration.amount.toString())

        return view
    }

    override fun updateStep(view: View, step: WaitStep) {
        val typeSpinner = view.findViewById<Spinner>(R.id.duration_type)
        val durationText = view.findViewById<EditText>(R.id.duration_count)
        step.duration.amount = durationText.text.toString().toShort()
        step.duration.type = DurationType.fromInt(typeSpinner.selectedItemId.toInt())!!
    }


}

