package com.vtavernier.cremote

import android.support.v7.widget.RecyclerView
import android.view.ContextMenu
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import com.vtavernier.cremote.models.Program
import com.vtavernier.cremote.models.Step

class StepAdapter(private val program: Program, private val itemClickListener: RecyclerViewClickListener) : RecyclerView.Adapter<StepAdapter.StepViewHolder>() {
    class StepViewHolder(private val program: Program, view: View, private val clickListener: RecyclerViewClickListener) : RecyclerView.ViewHolder(view), View.OnClickListener, View.OnCreateContextMenuListener {
        init {
            view.setOnClickListener(this)
            view.setOnCreateContextMenuListener(this)
        }

        val firstLine = view.findViewById<TextView>(R.id.first_line)
        val secondLine = view.findViewById<TextView>(R.id.second_line)
        val iconText = view.findViewById<TextView>(R.id.icon)

        override fun onClick(v: View?) {
            clickListener.onClick(v, adapterPosition)
        }

        override fun onCreateContextMenu(menu: ContextMenu?, v: View?, menuInfo: ContextMenu.ContextMenuInfo?) {
            clickListener.onCreateContextMenu(menu!!, v!!, menuInfo, adapterPosition)
        }

        fun bind(step: Step, position: Int) {
            firstLine.text = step.getFirstHeader()
            secondLine.text = step.getSubHeader(program)
            iconText.text = position.toString()
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): StepViewHolder {
        val view = LayoutInflater.from(parent.context)
                .inflate(R.layout.step_item, parent, false)

        return StepViewHolder(program, view, itemClickListener)
    }

    override fun onBindViewHolder(holder: StepViewHolder, position: Int) {
        holder.bind(program.steps[position], position)
    }

    override fun getItemCount(): Int {
        return program.steps.size
    }
}