package com.vtavernier.cremote

import android.content.Context
import android.os.ParcelUuid
import android.util.Log
import com.bubelich.jBaseZ85
import com.polidea.rxandroidble2.RxBleClient
import com.polidea.rxandroidble2.RxBleConnection
import com.polidea.rxandroidble2.RxBleDevice
import com.polidea.rxandroidble2.scan.ScanFilter
import com.polidea.rxandroidble2.scan.ScanSettings
import com.vtavernier.cremote.models.Program
import io.reactivex.Single
import io.reactivex.disposables.Disposable
import java.util.*

class GadgetManager(context: Context) {
    // BLE client object
    private val rxBleClient = RxBleClient.create(context)

    private fun RxBleConnection.sendCommand(command: String): Single<ByteArray>? {
        Log.d(TAG, "Sending command << $command >>")
        return this.writeCharacteristic(
                CHARACTERISTIC_UUID,
                command.toByteArray(Charsets.ISO_8859_1))
    }

    // Currently associated device
    private var rxBleDevice: RxBleDevice? = null

    fun selectDevice(callback: (RxBleDevice) -> Unit,
                     errorCallback: (Throwable) -> Unit) {
        rxBleDevice?.let {
            Log.d(TAG, "Using associated device " + it.macAddress)
            callback(it)
        } ?: run {
            Log.d(TAG, "Initializing scan for device")
            var scanSubscription: Disposable? = null

            scanSubscription = rxBleClient.scanBleDevices(ScanSettings.Builder()
                    .setCallbackType(ScanSettings.CALLBACK_TYPE_ALL_MATCHES)
                    .build(),
                    ScanFilter.Builder()
                            .setServiceUuid(SERVICE_UUID)
                            .build()
            ).subscribe({ scanResult ->
                scanResult.bleDevice?.let { bleDevice ->
                    Log.d(TAG, "Found device " + bleDevice.macAddress)
                    rxBleDevice = bleDevice

                    try {
                        callback(bleDevice)
                    } finally {
                        scanSubscription?.dispose()
                    }
                }
            }, errorCallback)
        }
    }

    fun connectDevice(callback: (RxBleConnection) -> Unit,
                      errorCallback: (Throwable) -> Unit): Disposable {
        return rxBleDevice!!.let { rxBleDevice ->
            rxBleDevice.establishConnection(false)
                    .subscribe({ rxBleConnection ->
                        Log.d(TAG, "Established connection with " + rxBleDevice.macAddress)
                        callback(rxBleConnection)
                    }, errorCallback)
        }
    }

    private fun getProgramDownloadCommands(program: Program): Collection<String> {
        val programBytes = program.toBytes()
        val commands = arrayListOf<String>()

        for (i in 0 until programBytes.size step 12) {
            // Last step if we end up past the program at next iteration
            val isLast = i + 12 >= programBytes.size
            val lastValueIndex =
                    when {
                        i + 12 <= programBytes.size -> 2
                        i + 8 <= programBytes.size -> 1
                        else -> 0
                    }
            val offset = i / 4

            // Build control word
            val control =
                    (((if (isLast) {
                        1
                    } else {
                        0
                    }) shl 7) or
                            (offset shl 2) or
                            lastValueIndex).toByte()

            // Build message
            val msgData = ByteArray((lastValueIndex + 1) * 4)
            System.arraycopy(programBytes, i, msgData, 0, msgData.size)

            // Encode message
            val encoded = jBaseZ85.encode(msgData)

            // Create command message
            // Use bytes to prevent Java/Kotlin messing with character casting
            val msgBytes = "CR+D.$encoded".toByteArray(Charsets.ISO_8859_1)
            msgBytes[4] = control
            val msg = msgBytes.toString(Charsets.ISO_8859_1)

            Log.d(TAG, "Command: $msg")

            // Write
            commands.add(msg)
        }

        return commands
    }

    fun uploadProgram(program: Program, rxBleConnection: RxBleConnection,
                      callback: () -> Unit, errorCallback: (Throwable) -> Unit) {
        var senderSubscription: Disposable? = null

        senderSubscription = rxBleConnection.sendCommand("CR+C")!!.flatMap {
            val commands = getProgramDownloadCommands(program)
            var observable = rxBleConnection.sendCommand(commands.first())

            commands.stream().skip(1).forEach { command ->
                observable = observable!!.flatMap {
                    rxBleConnection.sendCommand(command)
                }
            }

            observable
        }.subscribe({
            try {
                callback()
            } finally {
                senderSubscription?.dispose()
            }
        }, { throwable ->
            try {
                errorCallback(throwable)
            } finally {
                senderSubscription?.dispose()
            }
        })
    }

    companion object {
        val CHARACTERISTIC_UUID = UUID.fromString("0000ffe1-0000-1000-8000-00805f9b34fb")
        val SERVICE_UUID = ParcelUuid.fromString("0000ffe0-0000-1000-8000-00805f9b34fb")

        const val TAG: String = "GadgetManager"
    }
}
