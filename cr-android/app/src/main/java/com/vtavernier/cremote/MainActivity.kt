package com.vtavernier.cremote

import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothManager
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.os.Handler
import android.support.v4.app.ActivityCompat
import android.support.v4.content.ContextCompat
import android.support.v7.app.AppCompatActivity
import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.view.ContextMenu
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.widget.ProgressBar
import android.widget.Toast
import com.google.gson.JsonParseException
import com.vtavernier.cremote.fragments.EditLoopFragment
import com.vtavernier.cremote.fragments.EditPressFragment
import com.vtavernier.cremote.fragments.EditStepListener
import com.vtavernier.cremote.fragments.EditWaitFragment
import com.vtavernier.cremote.models.*
import io.reactivex.disposables.Disposable
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity(), EditStepListener {
    private lateinit var stepListView: RecyclerView
    private lateinit var stepListViewAdapter: RecyclerView.Adapter<*>
    private lateinit var stepListViewManager: RecyclerView.LayoutManager

    private lateinit var statusProgressBar: ProgressBar

    private var program = Program()

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
                showEditDialog(PressStep(StepDuration(DurationType.InverseSeconds, 60), StepDuration(DurationType.InverseSeconds, 2)), program.steps.size)
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
            program.steps.add(PressStep(StepDuration(DurationType.InverseSeconds, 60), StepDuration(DurationType.InverseSeconds, 2)))
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

        statusProgressBar = findViewById(R.id.status_progress_bar)
    }

    private fun deleteStep(position: Int) {
        program.steps.removeAt(position)
        stepListViewAdapter.notifyItemRemoved(position)
        stepListViewAdapter.notifyItemRangeChanged(position, program.steps.size - position)
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

    override fun getProgram(): Program {
        return program
    }

    override fun getInsertLocation(): Int {
        return currentPosition
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
            R.id.action_test -> {
                uploadProgram(false)
                true
            }
            R.id.action_upload -> {
                uploadProgram(true)
                true
            }
            R.id.action_launch -> {
                launchProgram()
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }

    private val mBluetoothAdapter: BluetoothAdapter? by lazy(LazyThreadSafetyMode.NONE) {
        val bluetoothManager = getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothManager.adapter
    }

    private val BluetoothAdapter.isDisabled: Boolean
        get() = !isEnabled

    private val gadgetManager by lazy {
        GadgetManager(this)
    }

    private var nextCallback: (() -> Unit)? = null

    private fun continueWithBluetooth(run: () -> Unit) {
        nextCallback = run

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_COARSE_LOCATION)
                != PackageManager.PERMISSION_GRANTED) {
            // Permission is not granted
            // Should we show an explanation?
            if (ActivityCompat.shouldShowRequestPermissionRationale(this,
                            Manifest.permission.ACCESS_COARSE_LOCATION)) {
                // Show an explanation to the user *asynchronously* -- don't block
                // this thread waiting for the user's response! After the user
                // sees the explanation, try again to request the permission.
            } else {
                // No explanation needed, we can request the permission.
                ActivityCompat.requestPermissions(this,
                        arrayOf(Manifest.permission.ACCESS_COARSE_LOCATION),
                        REQUEST_ACCESS_COARSE_LOCATION)
            }
        } else {
            nextCallback = null

            continueWithDevice(run)
        }
    }

    private fun continueWithDevice(run: () -> Unit) {
        // Ensures Bluetooth is available on the device and it is enabled. If not,
        // displays a dialog requesting user permission to enable Bluetooth. )
        mBluetoothAdapter?.apply {
            if (isDisabled) {
                val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
                startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT)
            } else {
                run()
            }
        }
    }

    private fun uploadProgram(persist: Boolean) {
        continueWithBluetooth { selectDeviceForAction { uploadProgramToDevice(persist) } }
    }


    private fun launchProgram() {
        continueWithBluetooth { selectDeviceForAction { launchProgramOnDevice() } }
    }

    private fun selectDeviceForAction(run: () -> Unit) {
        statusProgressBar.visibility = View.VISIBLE

        gadgetManager.selectDevice({ bleDevice ->
            Toast.makeText(this, bleDevice.macAddress, Toast.LENGTH_LONG).show()
            run()
        }, ::handleBleError)
    }

    private fun uploadProgramToDevice(persist: Boolean) {
        var connectionSubscription: Disposable? = null

        connectionSubscription = gadgetManager.connectDevice({ rxBleConnection ->
            gadgetManager.uploadProgram(program, persist, rxBleConnection, {
                Handler(mainLooper).post {
                    statusProgressBar.visibility = View.INVISIBLE
                    Toast.makeText(this, "Envoi terminé !", Toast.LENGTH_SHORT).show()
                    connectionSubscription?.dispose()
                }
            }, { throwable -> handleBleError(throwable, connectionSubscription) })
        }, { throwable -> handleBleError(throwable, connectionSubscription) })
    }

    private fun launchProgramOnDevice() {
        var connectionSubscription: Disposable? = null

        connectionSubscription = gadgetManager.connectDevice({ rxBleConnection ->
            gadgetManager.launchProgram(rxBleConnection, {
                Handler(mainLooper).post {
                    statusProgressBar.visibility = View.INVISIBLE
                    Toast.makeText(this, "Programme lancé !", Toast.LENGTH_SHORT).show()
                    connectionSubscription?.dispose()
                }
            }, { throwable -> handleBleError(throwable, connectionSubscription) })
        }, { throwable -> handleBleError(throwable, connectionSubscription) })
    }

    private fun handleBleError(throwable: Throwable, connectionSubscription: Disposable?) {
        try {
            handleBleError(throwable)
        } finally {
            connectionSubscription?.dispose()
        }
    }

    private fun handleBleError(throwable: Throwable) {
        Handler(mainLooper).post {
            statusProgressBar.visibility = View.INVISIBLE
            Toast.makeText(this, throwable.toString(), Toast.LENGTH_LONG).show()
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        if (requestCode == REQUEST_ENABLE_BT &&
                resultCode == RESULT_OK) {
            continueWithDevice(nextCallback!!)
        } else if (requestCode == REQUEST_ACCESS_COARSE_LOCATION) {
            if (resultCode == PackageManager.PERMISSION_GRANTED) {
                continueWithDevice(nextCallback!!)
            } else {
                Toast.makeText(this, "Envoi annulé", Toast.LENGTH_SHORT).show()
            }
        }
    }

    companion object {
        const val REQUEST_ENABLE_BT: Int = 1500
        const val REQUEST_ACCESS_COARSE_LOCATION: Int = 1501

        const val TAG: String = "MainActivity"
    }
}
