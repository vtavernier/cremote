package com.vtavernier.cremote.fragments

import android.app.AlertDialog
import android.app.Dialog
import android.content.Context
import android.os.Bundle
import android.support.v4.app.DialogFragment
import android.support.v4.app.FragmentActivity
import android.view.View
import com.vtavernier.cremote.R
import com.vtavernier.cremote.models.Step

abstract class BaseEditFragment<StepType : Step> : DialogFragment() {
    private lateinit var mListener: EditStepListener

    override fun onCreateDialog(savedInstanceState: Bundle?): Dialog {
        return activity?.let {
            // User the Builder class for convenient dialog construction
            val builder = AlertDialog.Builder(it)

            val listener = mListener
            val step = listener.getStep() as StepType
            val view = initView(it, step)

            builder.setTitle("Attendre")
                    .setView(view)
                    .setPositiveButton(if (listener.canDelete()) {
                        R.string.edit_fragment_edit_button
                    } else {
                        R.string.edit_fragment_add_button
                    }) { dialog, which ->
                        updateStep(view, step)
                        listener.completeEdit()
                    }
                    .setNegativeButton(R.string.edit_fragment_cancel_button) { dialog, which ->
                        dialog.cancel()
                    }

            if (listener.canDelete())
                builder.setNeutralButton(R.string.edit_fragment_remove_button) { dialog, which ->
                    listener.deleteEdit()
                }

            builder.create()
        } ?: throw IllegalStateException("Activity cannot be null")
    }

    override fun onAttach(context: Context?) {
        super.onAttach(context)
        mListener = context as EditStepListener
    }

    internal abstract fun initView(it: FragmentActivity, step: StepType): View
    internal abstract fun updateStep(view: View, step: StepType)
}