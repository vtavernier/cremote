package com.vtavernier.cremote.fragments

import android.content.Context
import android.support.v4.app.FragmentActivity
import android.view.LayoutInflater
import android.view.View
import android.widget.ArrayAdapter
import android.widget.Spinner
import android.widget.TextView
import com.vtavernier.cremote.R
import com.vtavernier.cremote.models.LoopStep
import com.vtavernier.cremote.models.Step

class EditLoopFragment : BaseEditFragment<LoopStep>("Boucler") {
    override fun initView(it: FragmentActivity, step: LoopStep): View {
        val view = (it.getSystemService(Context.LAYOUT_INFLATER_SERVICE) as LayoutInflater)
                .inflate(R.layout.edit_loop_fragment, null)!!

        val listener = getListener()
        val steps = listener.getProgram().steps.take(listener.getInsertLocation()).toTypedArray()

        setupStepSpinner(view, it, R.id.target_chooser, step, steps)
        setupTargetCount(view, it, R.id.target_count, step)

        return view
    }

    private fun setupStepSpinner(view: View, it: FragmentActivity, spinnerId: Int, step: LoopStep, steps: Array<Step>) {
        val spinner = view.findViewById<Spinner>(spinnerId)
        val adapter = ArrayAdapter(it, android.R.layout.simple_spinner_item, steps)

        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
        spinner.adapter = adapter
        spinner.setSelection(adapter.getPosition(step.targetStep))
    }

    private fun setupTargetCount(view: View, it: FragmentActivity, counterId: Int, step: LoopStep) {
        val counter = view.findViewById<TextView>(counterId)
        counter.text = step.count.toString(10)
    }

    override fun updateStep(view: View, step: LoopStep) {
        val spinner = view.findViewById<Spinner>(R.id.target_chooser)
        val counter = view.findViewById<TextView>(R.id.target_count)

        step.targetStep = spinner.selectedItem as Step

        try {
            step.count = Integer.parseInt(counter.text.toString()).toShort()
        } catch (ex: NumberFormatException) {
            step.count = 0
        }

        if (step.count < 1)
            step.count = 1
    }
}