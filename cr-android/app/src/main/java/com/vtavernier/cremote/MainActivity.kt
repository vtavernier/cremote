package com.vtavernier.cremote

import android.os.Bundle
import android.support.design.widget.Snackbar
import android.support.v7.app.AppCompatActivity
import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.view.Menu
import android.view.MenuItem
import android.view.View
import com.vtavernier.cremote.models.*
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {
    private lateinit var stepListView: RecyclerView
    private lateinit var stepListViewAdapter: RecyclerView.Adapter<*>
    private lateinit var stepListViewManager: RecyclerView.LayoutManager

    val program = Program()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        setSupportActionBar(toolbar)

        fab.setOnClickListener { view ->
            Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                    .setAction("Action", null).show()
        }

        program.steps.add(WaitStep(StepDuration(DurationType.Seconds, 5)))
        program.steps.add(PressStep(StepDuration(DurationType.Millis, 25)))

        stepListViewManager = LinearLayoutManager(this)
        stepListViewAdapter = StepAdapter(program, object : RecyclerViewClickListener {
            override fun onClick(view: View?, position: Int) {
                Snackbar.make(view!!, "Clicked: " + program.steps[position].toString(), Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show()
            }
        })

        stepListView = findViewById<RecyclerView>(R.id.step_list_view).apply {
            setHasFixedSize(true)

            layoutManager = stepListViewManager
            adapter = stepListViewAdapter
        }

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
        return when(item.itemId) {
            R.id.action_settings -> true
            else -> super.onOptionsItemSelected(item)
        }
    }
}
