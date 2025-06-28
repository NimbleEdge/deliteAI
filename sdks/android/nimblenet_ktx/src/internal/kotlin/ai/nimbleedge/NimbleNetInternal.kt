import ai.nimbleedge.controllers.InternalTaskController
import ai.nimbleedge.datamodels.NimbleNetResult
import ai.nimbleedge.impl.DependencyContainer
import android.app.Application

object NimbleNetInternal {
    @Volatile private lateinit var controller: InternalTaskController

    private fun getController(application: Application): InternalTaskController {
        return DependencyContainer.getInstance(null, null).getInternalTaskController().also {
            controller = it
        }
    }

    fun initialize(application: Application) {
        getController(application = application)
    }

    fun resetAppState() {
        controller.resetAppState()
    }

    fun reset() {
        controller.reset()
    }

    fun deleteDatabase() {
        controller.deleteDatabase()
    }

    fun reloadModelWithEpConfig(modelName: String, epConfig: String): NimbleNetResult<Unit> {
        return controller.reloadModelWithEpConfig(modelName, epConfig)
    }

    fun loadModelFromFile(
        modelFilePath: String,
        inferenceConfigFilePath: String,
        modelId: String,
        epConfigJson: String,
    ): NimbleNetResult<Unit> {
        return controller.loadModelFromFile(
            modelFilePath,
            inferenceConfigFilePath,
            modelId,
            epConfigJson,
        )
    }
}
