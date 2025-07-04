//package tasks
//
//import NEGradleConfig
//import com.android.build.gradle.api.LibraryVariant
//import com.android.build.gradle.internal.plugins.BasePlugin
//import org.gradle.api.AntBuilder
//import org.gradle.api.Project
//import org.gradle.api.publish.PublishingExtension
//import org.gradle.api.publish.maven.MavenPublication
//import org.gradle.api.publish.maven.internal.ant.AntSupport
//import org.gradle.api.publish.maven.plugins.MavenPublishPlugin
//import org.gradle.api.publish.maven.tasks.PublishToMavenRepository
//import org.gradle.kotlin.dsl.*
//import java.util.Properties
//
//class Publishing(private val project: Project, private val neGradleConfig: NEGradleConfig) {
//    fun apply() {
//        project.plugins.withType<MavenPublishPlugin> {
//            project.afterEvaluate {
//                project.extensions.configure<PublishingExtension> {
//                    publications {
//                        register<MavenPublication>("internalRelease") {
//                            from(project.components.getByName("internalRelease"))
//                            groupId = project.group.toString()
//                            artifactId = "nimblenet_ktx"
//                            version = neGradleConfig.releaseVersion
//                        }
//                        register<MavenPublication>("externalRelease") {
//                            from(project.components.getByName("externalRelease"))
//                            groupId = project.group.toString()
//                            artifactId = "nimblenet_ktx"
//                            version = neGradleConfig.releaseVersion
//                        }
//                    }
//                    repositories {
//                        maven {
//                            name = "deliteai_android"
//                            url = project.uri(project.getLocalProperty("AWS_S3_URL"))
//                            credentials(AwsCredentials::class) {
//                                accessKey = project.getLocalProperty("AWS_ACCESS_KEY_ID")
//                                secretKey = project.getLocalProperty("AWS_SECRET_ACCESS_KEY")
//                            }
//                        }
//                        maven {
//                            name = "ossrh"
//                            url = project.uri("https://ossrh-staging-api.central.sonatype.com/service/local/staging/deploy/maven2/")
//                            credentials {
//                                username = project.getLocalProperty("ossrhUsername")
//                                password = project.getLocalProperty("ossrhPassword")
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }
//}
