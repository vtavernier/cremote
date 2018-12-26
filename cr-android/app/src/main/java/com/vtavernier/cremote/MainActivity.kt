package com.vtavernier.cremote

import android.content.Context
import android.os.Bundle
import android.support.v7.app.AppCompatActivity
import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.view.ContextMenu
import android.view.Menu
import android.view.MenuItem
import android.view.View
import com.google.gson.JsonParseException
import com.vtavernier.cremote.fragments.EditLoopFragment
import com.vtavernier.cremote.fragments.EditPressFragment
import com.vtavernier.cremote.fragments.EditStepListener
import com.vtavernier.cremote.fragments.EditWaitFragment
import com.vtavernier.cremote.models.*
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity(), EditStepListener {
    private lateinit var stepListView: RecyclerView
    private lateinit var stepListViewAdapter: RecyclerView.Adapter<*>
    private lateinit var stepListViewManager: RecyclerView.LayoutManager

    var program = Program()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        setSupportActionBar(toolbar)

        fab.setOnClickListener { view ->
            view.showContextMenu()
            //Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG).setAction("Action", null).show()
        }

        fab.setOnCreateContextMenuListener { menu, v, menuInfo ->
            menu.setHeaderTitle("Ajouter")
            menu.add(0, v.id, 0, "Attendre...").setOnMenuItemClickListener {
                showEditDialog(WaitStep(StepDuration(DurationType.Seconds, 1)), program.steps.size)
                true
            }
            menu.add(0, v.id, 1, "Déclencher...").setOnMenuItemClickListener {
                showEditDialog(PressStep(StepDuration(DurationType.Millis, 33)), program.steps.size)
                true
            }
            menu.add(0, v.id, 2, "Boucler...").setOnMenuItemClickListener {
                showEditDialog(LoopStep(program.steps[0], 1), program.steps.size)
                true
            }
        }

        val sharedPref = getPreferences(Context.MODE_PRIVATE)
        val pref = sharedPref?.getString(getString(R.string.saved_program_key), null)
        if (pref != null) {
            try {
                program = Program.fromJson(pref)
            } catch (ex: JsonParseException) {
            }
        }

        if (program.steps.isEmpty()) {
            program.steps.add(WaitStep(StepDuration(DurationType.Seconds, 5)))
            program.steps.add(PressStep(StepDuration(DurationType.Millis, 33)))
            saveProgram()
        }

        stepListViewManager = LinearLayoutManager(this)
        stepListViewAdapter = StepAdapter(program, object : RecyclerViewClickListener {
            override fun onClick(view: View?, position: Int) {
                showEditDialog(program.steps[position], position)
            }

            override fun onCreateContextMenu(menu: ContextMenu, v: View, menuInfo: ContextMenu.ContextMenuInfo?, position: Int) {
                menu.setHeaderTitle(getString(R.string.main_activity_program_context_menu_title))
                menu.add(0, v.id, 0, "Modifier").setOnMenuItemClickListener {
                    showEditDialog(program.steps.get(position), position)
                    true
                }
                menu.add(0, v.id, 1, "Supprimer").setOnMenuItemClickListener {
                    deleteStep(position)
                    true
                }
            }
        })

        stepListView = findViewById<RecyclerView>(R.id.step_list_view).apply {
            setHasFixedSize(true)

            layoutManager = stepListViewManager
            adapter = stepListViewAdapter
        }

    }

    private fun deleteStep(position: Int) {
        program.steps.removeAt(position)
        stepListViewAdapter.notifyItemRemoved(position)
    }

    private lateinit var currentStep: Step
    private var currentPosition: Int = 0

    private fun showEditDialog(step: Step, position: Int) {
        currentStep = step
        currentPosition = position

        if (step is WaitStep) {
            val fragment = EditWaitFragment()
            fragment.show(supportFragmentManager, "add_wait_step")
        } else if (step is PressStep) {
            val fragment = EditPressFragment()
            fragment.show(supportFragmentManager, "add_press_step")
        } else if (step is LoopStep) {
            val fragment = EditLoopFragment()
            fragment.show(supportFragmentManager, "add_loop_step")
        }
    }

    private fun currentStepNew(): Boolean {
        return program.steps.size <= currentPosition || program.steps[currentPosition] !== currentStep
    }

    override fun completeEdit() {
        if (program.steps.size <= currentPosition) {
            program.steps.add(currentStep)
            stepListViewAdapter.notifyItemInserted(currentPosition)
        } else {
            if (program.steps[currentPosition] === currentStep) {
                stepListViewAdapter.notifyItemChanged(currentPosition)
            } else {
                program.steps.add(currentPosition, currentStep)
                stepListViewAdapter.notifyItemInserted(currentPosition)
            }
        }

        saveProgram()
    }

    private fun saveProgram() {
        val sharedPref = getPreferences(Context.MODE_PRIVATE) ?: return
        with(sharedPref.edit()) {
            putString(getString(R.string.saved_program_key), program.toJson())
            apply()
        }
    }

    override fun deleteEdit() {
        deleteStep(currentPosition)
    }

    override fun canDelete(): Boolean {
        return !currentStepNew()
    }

    override fun getStep(): Step {
        return currentStep
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        // Inflate the menu; this adds items to the action bar if it is present.
        menuInflater.inflate(R.menu.menu_main, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        return when (item.itemId) {
            R.id.action_settings -> true
            else -> super.onOptionsItemSelected(item)
        }
    }
}
