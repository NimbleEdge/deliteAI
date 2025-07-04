import com.android.build.api.dsl.DefaultConfig
import org.gradle.api.GradleException
import org.gradle.api.Project
import java.util.Properties

/**
 * Reads the specified keys from local.properties and injects them as buildConfigFields.
 */
fun DefaultConfig.addStringConfigsFromLocalProperties(
    keys: List<String>,
    project: Project
) = keys.forEach { key ->
    val value = project.getLocalProperty(key)
    buildConfigField("String", key, "\"$value\"")
}

/**
 * Loads a property from the root project's local.properties, throwing if missing.
 */
fun Project.getLocalProperty(key: String): String {
    val propsFile = rootProject.file("local.properties")
    val props = Properties().apply { if (propsFile.exists()) load(propsFile.inputStream()) }
    return props.getProperty(key)
        ?: throw GradleException("Missing local property: $key")
}
