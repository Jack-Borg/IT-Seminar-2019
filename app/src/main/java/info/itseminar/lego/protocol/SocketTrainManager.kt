package info.itseminar.lego.protocol

import android.os.AsyncTask
import android.os.AsyncTask.THREAD_POOL_EXECUTOR
import android.util.Log
import java.net.Socket

class SocketTrainManager() : TrainManager {
  var socket: Socket? = null
  override var train: Train? = null

  val connected: Boolean
    get() = train != null

  override fun connectAndListen(server: TrainServer, listener: (Command) -> Unit) {
    Log.d("TRAIN", "Connecting and listening")
    ConnectAndReceiveTask(this, listener).executeOnExecutor(THREAD_POOL_EXECUTOR, server)
    }

  override fun send(command: Command, handle: (String) -> Unit) {
    SendTask(this, handle).executeOnExecutor(THREAD_POOL_EXECUTOR, command)
    }

  class SendTask(val manager: SocketTrainManager, val handle: (String) -> Unit) : AsyncTask<Command, String, Boolean>() {

    override fun doInBackground(vararg commands: Command): Boolean {
      Log.d("TRAIN", "Send task started with ${commands.size} commands")
      for (command in commands) {
        try {
          Log.d("TRAIN", "Sending $command")
          val socket = manager.socket ?: throw Exception("Cannot retreive socket")
          command.to(socket.getOutputStream())
          publishProgress("$command send")
          }
        catch (e: Exception) {
          Log.e("TRAIN", "Error sending $command: ${e.message}")
          publishProgress("Error handling $command")
          return false
          }
        }
      return true
      }

    override fun onProgressUpdate(vararg values: String) {
      for (value in values) handle(value)
      }

    }

  class ConnectAndReceiveTask(val manager: SocketTrainManager, val handle: (Command) -> Unit) : AsyncTask<TrainServer, Command, Boolean>() {

    override fun doInBackground(vararg servers: TrainServer): Boolean {
      Log.d("TRAIN", "Connect and Receive task started")
      for (server in servers) {
        Log.d("TRAIN", "Trying to connect to $server")
        try {
          val socket = Socket(server.host, server.port)
          manager.socket = socket
          Log.d("TRAIN", "Connected to $server, starts listening")
          val input = socket.getInputStream()
          while (input != null) {
            val command = Command.from(input)
            // Log.d("TRAIN", "Received $command")
            if (command is Command.Nothing) {
              Log.d("TRAIN", "Nothing received, disconnecting...")
              return true
              }
            publishProgress(command)
            }
          return true
          }
        catch (e: Exception) {
          Log.e("TRAIN", "Can't connect to ${server.host} on ${server.port}")
          return false
          }
        }
      return true
      }

    override fun onProgressUpdate(vararg commands: Command) {
      for (command in commands) handle(command)
      }

    }

  }

