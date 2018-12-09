package com.vtavernier.cremote

import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import com.vtavernier.cremote.models.Program
import com.vtavernier.cremote.models.Step

interface RecyclerViewClickListener {
    fun onClick(view : View?, position : Int)
}

class StepAdapter(private val program: Program, private val itemClickListener: RecyclerViewClickListener) : RecyclerView.Adapter<StepAdapter.StepViewHolder>() {
    class StepViewHolder(private val view: View, private val clickListener: RecyclerViewClickListener) : RecyclerView.ViewHolder(view), View.OnClickListener {
        init {
            view.setOnClickListener(this)
        }

        val firstLine = view.findViewById<TextView>(R.id.first_line)
        val secondLine = view.findViewById<TextView>(R.id.second_line)

        override fun onClick(v: View?) {
            clickListener.onClick(v, adapterPosition)
        }

        fun bind(step: Step) {
            firstLine.text = step.javaClass.name
            secondLine.text = step.toString()
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): StepViewHolder {
        val view = LayoutInflater.from(parent.context)
                .inflate(R.layout.step_item, parent, false)

        return StepViewHolder(view, itemClickListener)
    }

    override fun onBindViewHolder(holder: StepViewHolder, position: Int) {
        holder.bind(program.steps[position])
    }

    override fun getItemCount(): Int {
        return program.steps.size
    }
}