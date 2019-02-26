package com.vtavernier.cremote.fragments

import android.content.Context
import android.support.v4.app.FragmentActivity
import android.view.LayoutInflater
import android.view.View
import android.widget.ArrayAdapter
import android.widget.Spinner
import com.vtavernier.cremote.R
import com.vtavernier.cremote.models.PressStep
import com.vtavernier.cremote.models.StepDuration

class EditPressFragment : BaseEditFragment<PressStep>("Déclencher") {
    override fun initView(it: FragmentActivity, step: PressStep): View {
        val view = (it.getSystemService(Context.LAYOUT_INFLATER_SERVICE) as LayoutInflater)
                .inflate(R.layout.edit_press_fragment, null)

        setupDurationSpinner(view, it, R.id.press_duration, step.duration)
        setupDurationSpinner(view, it, R.id.halfpress_duration, step.halfPressDuration)

        return view
    }

    private fun setupDurationSpinner(view: View, it: FragmentActivity, spinnerId: Int, targetDuration: StepDuration) {
        val durationSpinner = view.findViewById<Spinner>(spinnerId)
        ArrayAdapter.createFromResource(
                it,
                R.array.press_duration_names,
                android.R.layout.simple_spinner_item
        ).also { adapter ->
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
            durationSpinner.adapter = adapter
            durationSpinner.setSelection(adapter.getPosition(targetDuration.toShutterSpeed()))
        }
    }

    override fun updateStep(view: View, step: PressStep) {
        view.findViewById<Spinner>(R.id.press_duration).let { durationSpinner ->
            step.duration = StepDuration.parse(durationSpinner.selectedItem as String)!!
        }

        view.findViewById<Spinner>(R.id.halfpress_duration).let { halfpressDurationSpinner ->
            step.halfPressDuration = StepDuration.parse(halfpressDurationSpinner.selectedItem as String)!!
        }
    }
}

