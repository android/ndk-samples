
- [Introduction](#introduction)
- [Configure NDK for Android Gradle Plugins](#configure-ndk-for-android-gradle-plugins)
  * [NDK with AGP 4.1](#ndk-with-agp-41)
    + [Under the hood](#under-the-hood)
  * [NDK with AGP 4.0 and 3.6](#ndk-with-agp-40-and-36)
    + [Under the hood](#under-the-hood-1)
  * [NDK with AGP 3.5](#ndk-with-agp-35)
    + [Under the hood](#under-the-hood-2)
  * [NDK with AGP 3.4](#ndk-with-agp-34)
- [NDK Version Specification](#ndk-version-specification)
- [NDK Release Channels](#ndk-release-channels)
- [Install NDKs](#install-ndks)
  * [SDK Manager UI](#sdk-manager-ui)
  * [The sdkmanager command line tool](#the-sdkmanager-command-line-tool)
  * [Manually install NDK](#manually-install-ndk)
- [Terminologies](#terminologies)
  * [ndkVersion](#ndkversion)
  * [non-ndkVersion](#non-ndkversion)
  * [ndk.dir](#ndkdir)
  * [ANDROID\_NDK\_HOME](#android--ndk--home)
  * [The default NDK version](#the-default-ndk-version)
  * [The default NDK location](#the-default-ndk-location)



# Introduction

This document guides developers through NDK configuration in recent [Android Gradle Plugin (AGP)](https://google.github.io/android-gradle-dsl) versions. For Android Studio, this document assumes it uses the same version of Android Gradle Plugin (AGP).

The possible directories that AGP locates NDK are:


<table>
  <tr>
   <td>
   </td>
   <td colspan="4" ><strong>Android Studio/Gradle Plugin Version</strong>
   </td>
  </tr>
  <tr>
   <td>
   </td>
   <td><strong>4.1</strong>
   </td>
   <td><strong>4.0 - 3.6</strong>
   </td>
   <td><strong>3.5</strong>
   </td>
   <td><strong>3.4</strong>
   </td>
  </tr>
  <tr>
   <td><strong>Default NDK Location</strong>
   </td>
   <td>$SDK/ndk/$version
<p>
$SDK/ndk-bundle
   </td>
   <td>$SDK/ndk/$version
<p>
$SDK/ndk-bundle
   </td>
   <td>$SDK/ndk/$version
<p>
$SDK/ndk-bundle
   </td>
   <td>$SDK/ndk-bundle
   </td>
  </tr>
  <tr>
   <td><strong>ndkPath</strong>
   </td>
   <td>available
   </td>
   <td>Not available
   </td>
   <td>Not available
   </td>
   <td>Not available
   </td>
  </tr>
  <tr>
   <td><strong>ANDROID_NDK_HOME</strong>
   </td>
   <td>removed
   </td>
   <td>deprecated
   </td>
   <td>deprecated
   </td>
   <td>available
   </td>
  </tr>
  <tr>
   <td><strong>ndk.dir</strong>
   </td>
   <td>deprecated
   </td>
   <td>available
   </td>
   <td>available
   </td>
   <td>available
   </td>
  </tr>
</table>


The related available features that could affect which version of NDK to use are:


<table>
  <tr>
   <td>
   </td>
   <td colspan="5" ><strong>Android Studio/Gradle Plugin Version</strong>
   </td>
  </tr>
  <tr>
   <td>
   </td>
   <td><strong>4.1</strong>
   </td>
   <td><strong>4.0</strong>
   </td>
   <td><strong>3.6</strong>
   </td>
   <td><strong>3.5</strong>
   </td>
   <td><strong>3.4</strong>
   </td>
  </tr>
  <tr>
   <td><strong>ndkVersion</strong>
   </td>
   <td colspan="4" >defines specific NDK version
   </td>
   <td>Not available
   </td>
  </tr>
  <tr>
   <td><strong>Default NDK Version</strong>
   </td>
   <td colspan="3" >assign one NDK version by AGP
   </td>
   <td>Not available
   </td>
   <td>Not available
   </td>
  </tr>
  <tr>
   <td><strong>Autodownload</strong>
   </td>
   <td>$default & $ndkVersion
   </td>
   <td>$ndkVersion
   </td>
   <td>not available
   </td>
   <td>not available
   </td>
   <td>not available
   </td>
  </tr>
</table>


Out of the NDK path configuration channels, 

[ndkVersion](#ndkversion) feature added in version 3.5 and above provides a major flexibility for applications: NDK dependency moves from project level to module level, and we strongly recommend developers to take full advantage of the feature.

With the addition of [android.ndkVersion](#ndkversion), Android Studio may work in 2 ways:



*   [ndkVersion](#ndkVersion) way:  where `android.ndkVersion` is used in the application
*   [legacy ](#non-ndkVersion)way: NDK version does not matter to your application -- any one would work

The motivation for the evolutions is:



*   NDK usage is being integrated into build.gradle DSL
*   let [sdkmanager](https://developer.android.com/studio/command-line/sdkmanager) tool to manage NDK installation


# Configure NDK for Android Gradle Plugins

This section outlines the NDK path resolution with regard to recent Android Studio/Android Gradle Plugin versions; assuming that [Android Studio](https://developer.android.com/studio) uses the same version of AGP, the instructions directly apply to Android Studio IDE.


## NDK with AGP 4.1
With AGP 4.1, NDK path configuration is automatically done with:
*   if you need a specific NDK for any module, use <code>[ndkVersion](#ndkVersion)</code> in module’s `build.gradle`
*   otherwise, make sure do not use <code>ndkVersio</code> and AGP will behave as if <code>ndkVersion</code> was set to [the default NDK version](#the-default-ndk-version)

If you have the needed NDK versions on your local system, it will be used; otherwise, AGP 4.1 will automatically download it to the <code>$SDK/ndk/$version</code> and proceed to build.

That is all for AGP 4.1! If you want to know more, read onto the next section.

### Under the hood

AGP 4.1.x, added the <code>[android.ndkPath](#non-ndkversion)</code> to support some special usage cases such as that your own NDK must be in certain locations or even have your own customized NDK; on the other hand, AGP 4.1 deprecates <code>ndk.dir</code> and <code>$SDK/ndk-bundle</code>. Taking all of the possible ways into the consideration, AGP 4.1 searches the needed NDK in the following directories:

<table>
  <tr>
   <td>NDK Location</td>
   <td> if using<a href="#ndkVersion"> ndkVersion</a></td>
   <td>if not using<a href="#ndkVersion"> ndkVersion</a></td>
  </tr>
  <tr>
   <td><a href="#the-default-ndk-location">$SDK/ndk/${ndk-version}</a></td>
   <td> the exact version</td>
   <td><a href="#the-default-ndk-version">the internal default NDK Version</a></td>
  </tr>
  <tr>
   <td><a href="#the-default-ndk-location">$SDK/ndk-bundle</a></td>
   <td>the exact version(deprecated)</td>
   <td><a href="#the-default-ndk-version">the internal default NDK version (deprecated) </a></td>
  </tr>
  <tr>
   <td><a href="#non-ndkversion">android.ndkPath</a></td>
   <td>Not Used</td>
   <td>any version</td>
  </tr>
  <tr>
   <td><a href="#ndkdir">ndk.dir</a></td>
   <td>the exact version (deprecated)</td>
   <td>any version(deprecated)</td>
  </tr>
</table>

With the above, you could use any NDK locations to configure your local development system to fit your special needs.

## NDK with AGP 4.0 and 3.6

The correct way is to use NDK is <code>[ndkVersion](#ndkversion)</code> way: 



*   configure your NDK version to <code>

<p id="gdcalert18" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "android.ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert19">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[android.ndkVersion](#heading=h.it7m4v14uo8v)</code> in module’s <code>build.gradle;</code>
*   if building from command line with gradle, you need to 

<p id="gdcalert19" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "install the NDK"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert20">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[install the NDK](#heading=h.4ovddc691s0p)

The location for NDK is <code>$SDK/ndk/$ndk-version. </code>One noticeable difference is that AGP 4.0, when used inside Android Studio IDE, will auto download the needed NDK version if it is NOT installed locally; you need to 

<p id="gdcalert20" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "install NDK"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert21">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[install NDK](#heading=h.4ovddc691s0p) yourself with AGP 3.6.


### Under the hood

For the apps that do not use 

<p id="gdcalert21" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert22">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[ndkVersion](#heading=h.it7m4v14uo8v), AGP 3.6 and later versions embed a 

<p id="gdcalert22" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "default NDK version"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert23">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[default NDK version](#heading=h.tai9kfwyjxzi), which is the NDK version used to test this AGP version at the release time. The purpose is to offer a “known good” NDK for applications that does not specify any particular NDK.

Including the 

<p id="gdcalert23" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "default NDK version"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert24">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[default NDK version](#heading=h.tai9kfwyjxzi), AGP 3.6+ searches NDK in the following locations for the needed version: 


<table>
  <tr>
   <td>Location
   </td>
   <td>using 

<p id="gdcalert24" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert25">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

<a href="#heading=h.it7m4v14uo8v">ndkVersion</a>
   </td>
   <td>without 

<p id="gdcalert25" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert26">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

<a href="#heading=h.knmj1qgf5tj0">ndkVersion</a>
   </td>
  </tr>
  <tr>
   <td>$SDK/ndk/${ndk-version}
   </td>
   <td> required version
   </td>
   <td>

<p id="gdcalert26" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "the internal default"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert27">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

<a href="#heading=h.tai9kfwyjxzi">the internal default</a>
<p>


<p id="gdcalert27" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "AGP ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert28">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

<a href="#heading=h.tai9kfwyjxzi">AGP ndkVersion</a>
   </td>
  </tr>
  <tr>
   <td>$SDK/ndk-bundle
   </td>
   <td>required version
<p>
(deprecated)
   </td>
   <td>

<p id="gdcalert28" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "the internal default"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert29">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

<a href="#heading=h.tai9kfwyjxzi">the internal default</a>
<p>


<p id="gdcalert29" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "AGP ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert30">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

<a href="#heading=h.tai9kfwyjxzi">AGP ndkVersion</a>
<p>
(deprecated)
   </td>
  </tr>
  <tr>
   <td>ndk.dir
   </td>
   <td>required version
   </td>
   <td>any version
   </td>
  </tr>
</table>


The above behaviour provides some flexibility when developers have to customize NDK path.


## NDK with AGP 3.5

Android Gradle Plugin(AGP) 3.5.x added the 

<p id="gdcalert30" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert31">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[ndkVersion](#heading=h.it7m4v14uo8v) feature (also called NDK “side by side”, SxS, means multiple NDKs could be installed under the same directory) to support NDK dependency at module level: different modules could use different NDKs to build a single application or multiple applications. `ndkVersion` creates a new NDK default location at `$SDK/ndk/$version`:

Under `ndkVersion` way, NDKs are expected to be installed in a 

<p id="gdcalert31" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "new default location"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert32">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[new default location](#heading=h.6h0g5yn2ew0j)


```
    $SDK/ndk/${ndk-version-1}
    $SDK/ndk/${ndk-version-2}
```


Refer to 

<p id="gdcalert32" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert33">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[ndkVersion](#heading=h.it7m4v14uo8v) for additional information about the feature.

From AGP 3.5, developers are recommended to use the 

<p id="gdcalert33" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert34">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[ndkVersion](#heading=h.it7m4v14uo8v):



*   configure your NDK version to `android.ndkVersion` in module’s `build.gradle`
*   with Android Studio, the required NDK will be prompted to download automatically if not there 
*   if using command line environment or  CI, you need to 

<p id="gdcalert34" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "install the NDK"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert35">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[install the NDK](#heading=h.4ovddc691s0p)

and that is all to get NDK working with Android Gradle Plugin/Android Studio 3.5.


### Under the hood

With this great 

<p id="gdcalert35" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert36">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[ndkVersion](#heading=h.it7m4v14uo8v) considered, AGP 3.5 finds the matching NDK in the following locations:


<table>
  <tr>
   <td>NDK Location
   </td>
   <td>with 

<p id="gdcalert36" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert37">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

<a href="#heading=h.it7m4v14uo8v">ndkVersion</a>
   </td>
   <td>

<p id="gdcalert37" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "non-ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert38">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

<a href="#heading=h.knmj1qgf5tj0">non-ndkVersion</a> way
   </td>
  </tr>
  <tr>
   <td>

<p id="gdcalert38" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "$SDK/ndk/${ndk-version}"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert39">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

<a href="#heading=h.6h0g5yn2ew0j">$SDK/ndk/${ndk-version}</a>
   </td>
   <td>required version
   </td>
   <td>the newest NDK there
   </td>
  </tr>
  <tr>
   <td>

<p id="gdcalert39" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "$SDK/ndk-bundle"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert40">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

<a href="#heading=h.6h0g5yn2ew0j">$SDK/ndk-bundle</a>
   </td>
   <td>required<strong> </strong>version
   </td>
   <td>any version
   </td>
  </tr>
  <tr>
   <td>

<p id="gdcalert40" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "ndk.dir"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert41">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

<a href="#heading=h.c9m4jol5pqd">ndk.dir</a>
   </td>
   <td>required version
   </td>
   <td>any version
   </td>
  </tr>
</table>



## NDK with AGP 3.4

Android Gradle Plugin 3.4 is legacy now, configuring NDK path is relatively fixed and straightforward:



*   <code>

<p id="gdcalert41" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "ndk.dir"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert42">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[ndk.dir](#heading=h.c9m4jol5pqd)</code> in local.properties
*   <code>ANDROID_NDK_HOME</code> environment variable
*   <code>

<p id="gdcalert42" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "$SDK/ndk-bundle "). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert43">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[$SDK/ndk-bundle ](#heading=h.6h0g5yn2ew0j)</code>(default location for Studio 3.4)

Refer to “

<p id="gdcalert43" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "Install NDK"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert44">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[Install NDK](#heading=h.4ovddc691s0p)” for NDK installation details.


# NDK Version Specification

There are 3 parts to NDK version number as `$major.$minor.$build-$suffix`, for example, NDK `21.0.6011959-beta2`:



*   major version, i.e. 21
*   minor version, i.e. 0
*   build number,  i.e. `6113669`
*   suffix, i.e. “`beta2`”, 

The NDK version could be found in the ${ndk}/source.properties file; however when looking at the SDKManager or using sdkmanager command line tool, the suffix string, follows “rc”(release candidate) scheme, for example, **the above version** would become


      “`21.0.6011959 rc2"`

The required NDK version format in <code>

<p id="gdcalert44" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "android.ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert45">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[android.ndkVersion](#heading=h.it7m4v14uo8v)</code> are AGP version dependent:


<table>
  <tr>
   <td>AGP Version
   </td>
   <td>$major.$minor.$build$suffix
   </td>
   <td>$major.$minor.$build
   </td>
   <td>$major.$minor
   </td>
  </tr>
  <tr>
   <td>4.1 / 4.0
   </td>
   <td rowspan="3" >“<code>21.0.6011959 rc2"</code>
<p>
“<code>21.0.6011959-beta2"</code>
<p>
<code>all works!</code>
   </td>
   <td><code>"21.0.6011959" works</code>
   </td>
   <td>NO
   </td>
  </tr>
  <tr>
   <td>3.6
   </td>
   <td><code>NO</code>
   </td>
   <td>NO
   </td>
  </tr>
  <tr>
   <td>3.5
   </td>
   <td><code>NO</code>
   </td>
   <td><code>"21.0" works</code>
   </td>
  </tr>
</table>


Recommendation is:



*   $major.$minor.$build:  for AGP version 4.0/4.1, ignore the lengthy suffix format
*   $major.$minor.$build$suffix:  AGP 3.5/3.6 have to use the full version, all legal suffixes string would work ( “beta”, “rc”)

regex format is not supported!


# NDK Release Channels

Similar to other SDK components, NDKs are also released through different channels:



*   Stable for stable releases
*   Beta for beta release
*   Canary including developer preview NDKs

Formal releases could be installed with all installation channels, but the developer preview releases are only installable through Android SDK Manager (UI or command line tool) that has a **canary** channel.


# Install NDKs

The preferred way is to let AGP/Android Studio (4.0+) to auto download NDK into the `$SDK/ndk/$version.`

For AGP versions that could not auto-download NDK, there are at least 3 common ways available to install NDK, probably the easiest one is to use the [sdkmanager](https://developer.android.com/studio/command-line/sdkmanager) tool (UI or the command line version) shipped inside Android SDK for best compatibility purpose with your AGP version.


## SDK Manager UI

Before using SDK Manager, if you need to install NDKs only available on canary channel, you would need to turn it on. Canary Channel could be enabled inside Android Studio IDE:



*   Preferences > Appearance & Behavior > System Settings > Updates \


<p id="gdcalert45" ><span style="color: red; font-weight: bold">>>>>  GDC alert: inline image link here (to images/Untitled-document0.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert46">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>


![alt_text](images/Untitled-document0.png "image_tooltip")


SDK Manager GUI is integrated into Android Studio IDE. You could start SDK Manager GUI within Android Studio IDE:



1.  menu Tools > SDK Manager, or use “SDK Manager Toolkit”

<p id="gdcalert46" ><span style="color: red; font-weight: bold">>>>>  GDC alert: inline image link here (to images/Untitled-document1.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert47">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>


![alt_text](images/Untitled-document1.png "image_tooltip")

1.  left hand pane, “Appearance & Behaviour” > “System Settings” > “Android SDK”
1.  tab “SDK Tools”
1.  enable “Show Package Details” at the lower right corner \


<p id="gdcalert47" ><span style="color: red; font-weight: bold">>>>>  GDC alert: inline image link here (to images/Untitled-document2.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert48">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>


![alt_text](images/Untitled-document2.png "image_tooltip")

1.  Select NDK
    1.  for Android Studio 3.5.0+, install “Side by Side NDK”
        1.   expand “NDK(Side by Side)”
        1.  select the right version you need

        NDK will be installed to `${sdk}/ndk/${selected_version}`

    1.  for Android Studio versions before 3.5.0, select “NDK”: \
    the **latest NDK at the time** will be picked up automatically, and installed to `${sdk}/ndk-bundle`
1.  “Apply” or “OK” button to install NDK.


## The sdkmanager command line tool

[The sdkmanager command line tool](https://developer.android.com/studio/command-line/sdkmanager) shipped inside SDK could also be used to install all NDKs; for some scenarios like CI environment, the command line tool might be more helpful.  The generic syntax for NDK purpose is:


```
${sdk}/tools/bin/sdkmanager --install "ndk-selection"  --channel=${channel}
```


the relevant options are



*   ndk-selection:
    *   “`ndk;${your-ndk-version}`”: for “side by side” NDK; installed location is `$SDK/ndk/${your-ndk-version}`
    *   “`ndk-bundle`” for legacy ndk-bundle way; installed location is `${SDK}/ndk-bundle`
*   channels for NDK
    *   0: Stable NDK
    *   1: beta NDK
    *   3: canary NDK, including Developer Preview NDKs

To view how many NDK versions are available, use `"--list"` option:


```
    ${sdk}/tools/bin/sdkmanager --channel=${channel} --list
```


For additional sdkmanager usages, refer to[ the formal documentation](https://developer.android.com/studio/command-line/sdkmanager).


## Manually install NDK

For some reason you need to install NDK manually, you could do that:



*   download NDK from [official NDK site](https://developer.android.com/ndk/downloads)
*   unzip it to `$SDK/ndk/`

There might be differences comparing to the SDK Manager way to install, for example



*   currently you may not manually download canary NDKs


# Terminologies


## ndkVersion

In order to support multiple NDKs co-existing:



*   different NDKs to build different modules in one project
*   different NDKs to build different projects inside Studio IDE

Android Studio 3.5+ implemented the module level `android.ndkVersion` gradle DSL, for example in app/build.gradle :


```
    android {
    ndkVersion "21.1.6273396"
}
```


`ndkVersion `is the modern NDK SxS** (**side by side) way for Android Studio:



*   NDK SxS = `android.ndkVersion`: it is (still relatively new and) modern Studio way.

Otherwise Studio works in the old fashioned legacy mode. Please refer to 

<p id="gdcalert48" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "NDK Version Specification"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert49">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[NDK Version Specification](#heading=h.nkin2vvlotez) for specific AGP version’s requirement about version string format!


## non-ndkVersion

If application does not use the new <code>

<p id="gdcalert49" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert50">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[ndkVersion](#heading=h.it7m4v14uo8v)</code> feature, Studio is working in the <strong>legacy mode</strong>:



*   legacy mode = !android.ndkVersion

We strongly recommend developers to use the modern 

<p id="gdcalert50" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert51">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[ndkVersion](#heading=h.it7m4v14uo8v) way!

Android Studio 4.1+ added a new way to customize NDK location with `android.ndkPath`, for example, in module` build.gradle`, you could simply do:


```
    android {
    ndkPath "/Users/ndkPath/ndk21"  // pointing to your own NDK
}
```


please remember to **remove** ndkPath before distributing your source code: _it should be left outside your version control system_.


## ndk.dir

Android Studio creates file `local.properties` at the root of the project to save project level settings including NDK and SDK paths, for example:



*   `sdk.dir=/Users/ndkPath/dev/sdk`
*   `ndk.dir=/Users/ndkPath/dev/latest_ndk`

The cached NDK path is called **<code>ndk.dir</code></strong>, normally it is the first place that AGP/Android Studio looks for NDK; on the other hand, <code>ndk.dir </code>is deprecated from AGP 4.1.

The value of `ndk.dir `may come from 3 sources:



*   **ANDROID\_NDK\_HOME** environment variable when Studio starts up
*   **“Android NDK location” **within Android Studio IDE
*   brutally editing `ndk.dir` in `local.properties` file

“**Android NDK location**” in Android Studio IDE would always be in sync with ndk.dir: change one, the other will be mirrored automatically; you could find “Android NDK location” under “File” > “Project Structure” > “Android NDK location”:


    

<p id="gdcalert51" ><span style="color: red; font-weight: bold">>>>>  GDC alert: inline image link here (to images/Untitled-document3.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert52">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>


![alt_text](images/Untitled-document3.png "image_tooltip")


Cautions for ndk.dir:



1.  `ndk.dir`’s setting is local to your system: file `local.properties` is Studio generated file; as the name implies, it is local to your development machine , and should not be distributed with source code.
1.  `ndk.di`r is being deprecated from AGP 4.1+.
1.  The **ANDROID\_NDK\_HOME** environment variable is deprecated from AGP 3.6+.


## ANDROID\_NDK\_HOME

The ANDROID\_NDK\_HOME environment setting is the oldest of all NDK path configurations: an antique! It is the development system wise configuration: once set, it applies to all -- Android Studio reads this settings and caches it to UI/`ndk.dir` at starting up:



*   $ANDROID\_NDK\_HOME → ndk.dir/Studio IDE
*   ANDROID\_NDK\_HOME is old and deprecated

The replacement for ANDROID\_NDK\_HOME is the modern_ 

<p id="gdcalert52" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "ndkVersion way"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert53">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[ndkVersion way](#heading=h.it7m4v14uo8v)_: configure NDK in your modules, and install NDK to `$SDK/ndk/$version`

For CI purposes, the environmental variable ANDROID\_NDK\_HOME(deprecated) might be useful when building on the command line



*   Gradle does check ANDROID\_NDK\_HOME to find NDK if no local.properties file exists.
*   You could make ANDROID\_NDK\_HOME into your project’s ndkPath, for example, in module gradle file: \
<code>ndkPath = System.<em>getenv</em>(<strong>"ANDROID_NDK_HOME"</strong>)</code>


## The default NDK version

AGP 3.6+ embeds a default NDK version (_ANDROID\_GRADLE\_PLUGIN\_FIXED\_DEFAULT\_NDK\_VERSION_

), the “known-good” version when a specific AGP was released for application to use if application does not specify a particular NDK version:



*   the default version will not be used when application uses 

<p id="gdcalert53" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert54">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[ndkVersion](#heading=h.it7m4v14uo8v) feature, and 
*   it will not be used if application uses `ndk.dir` inside` local.properties`

Please be reminded that each AGP version has its own embedded default NDK version number.

Once the default NDK version is triggered, the application behaves as if 

<p id="gdcalert54" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert55">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[ndkVersion](#heading=h.it7m4v14uo8v) were set with the following:


```
    android.ndkVersion "${the-default-NDK-version}"
```


From here, all <code>

<p id="gdcalert55" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "android.ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert56">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[android.ndkVersion](#heading=h.it7m4v14uo8v)</code> path searching rules apply, check each AGP version in this documentation about the designed behaviors.


## The default NDK location

Unless Android Studio/Android Gradle Plugin(AGP) is purposely configured to go to a specific local directory for NDK, the default NDK locations will be used; in that sense, the default NDK locations are behind the customized NDK location(s).

The exact default NDK location depends on AGP version:



*   $SDK/ndk/: This is the new location for <code>

<p id="gdcalert56" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert57">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[ndkVersion](#heading=h.it7m4v14uo8v)</code> way. Multiple NDKs could be installed under this default NDK location, like:
    *   <code>$SDK/ndk/$version1</code>
    *   <code>$SDK/ndk/$version2</code>

    The subdirectory names under <code>$SDK/ndk</code> could be anything, <code>sdkmanager</code> tool by default chooses to use the <code>${version number}</code>s as sub-directory names, but you could name them freely: AGP finds the version string in <code>source.properties</code> packed inside NDK.

*   $SDK/ndk-bundle: existed before <code>

<p id="gdcalert57" ><span style="color: red; font-weight: bold">>>>>  GDC alert: undefined internal link (link text: "ndkVersion"). Did you generate a TOC? </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert58">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>> </span></p>

[ndkVersion](#heading=h.it7m4v14uo8v)</code> was added. It could only host one NDK version and is deprecated now but still part of the AGP’s NDK search path. Refer to the specific AGP version section for the deprecation status.
