package com.google.sample.echo;

import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.content.pm.PackageManager;
import android.support.annotation.NonNull;
import android.support.design.widget.Snackbar;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

/**
 * Created by wilkinsonclay on 2/1/16.
 */
public class PermissionRequestFragment extends Fragment {
    private static final String TAG = "PermReqFragment";
    private static final int RC_REQUEST_PERMISSION = 123;

    // These are error codes
    public static final int Error_FragmentNotAttached = -2;
    public static final int Error_Busy = -3;

    private TextView status_view;
    private boolean playing;

    private static PermissionRequestFragment theInstance;

    private long callbackPtr;


    /**
     * Called when a fragment is first attached to its context.
     *
     *
     * @param context - the context for this fragment.
     */
    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        theInstance = this;
        callbackPtr = 0L;
    }

    /**
     * Static method callable from native code.  This method checks the permission, and then
     * invokes the callback with the results.  This callback may happen immediately if the permission
     * state is already known, or it may need to go through the requesting permission and onRequestPermissionsResult
     * before the callback is called.
     * <p>If there is already a pending permission check, the callback is invoked immediately  with
     * and error.</p>
     * @param permission - the permission to check.
     * @param callbackPtr - the pointer to the callback function which has the signature (jint), the
     *                    value is either Packagemanager.PERMISSION_GRANTED (0),
     *                    Packagemanager.PERMISSION_DENIED (-1), or another negative number indicating
     *                    an error state.
     */
    public static void checkPermission(final String permission, String rationale, long callbackPtr)
    {
        if (theInstance == null) {
            Log.e(TAG,"An instance of this fragment has not been attached to the activity.");

           handlePermissionResult(Error_FragmentNotAttached, callbackPtr);
            return;
        }
        if (theInstance.callbackPtr != 0L) {
            Log.e(TAG,"Permission check already in progress");
            handlePermissionResult(Error_Busy, callbackPtr);
            return;
        }

        theInstance.callbackPtr = callbackPtr;

        int permissionCheck = ContextCompat.checkSelfPermission(theInstance.getActivity(), permission);
        if (permissionCheck == PackageManager.PERMISSION_DENIED) {
            // request runtime permission
            if (ActivityCompat.shouldShowRequestPermissionRationale(theInstance.getActivity(),
                    permission)) {

                final Activity thisActivity = theInstance.getActivity();
                View view = thisActivity.getCurrentFocus();
                if (view == null) {
                    view = thisActivity.getWindow().getDecorView();
                }
                Snackbar.make(view,
                        rationale, //"This sample requires permission to record audio",
                        Snackbar.LENGTH_INDEFINITE).setAction("OK", new View.OnClickListener() {
                    @Override
                    public void onClick(View view1) {
                        theInstance.requestPermissions(new String[] {permission}, RC_REQUEST_PERMISSION);
                    }
                }).show();
            } else {
                theInstance.requestPermissions(new String[] {permission}, RC_REQUEST_PERMISSION);
            }


        } else {
            long ptr = theInstance.callbackPtr;
            theInstance.callbackPtr = 0L;
           handlePermissionResult(PackageManager.PERMISSION_GRANTED, ptr);
        }

    }

    /**
     * Callback for the result from requesting permissions. This method
     * is invoked for every call on {@link #requestPermissions(String[], int)}.
     * <p>
     * <strong>Note:</strong> It is possible that the permissions request interaction
     * with the user is interrupted. In this case you will receive empty permissions
     * and results arrays which should be treated as a cancellation.
     * </p>
     *
     * @param requestCode  The request code passed in {@link #requestPermissions(String[], int)}.
     * @param permissions  The requested permissions. Never null.
     * @param grantResults The grant results for the corresponding permissions
     *                     which is either {@link PackageManager#PERMISSION_GRANTED}
     *                     or {@link PackageManager#PERMISSION_DENIED}. Never null.
     * @see #requestPermissions(String[], int)
     */
    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           @NonNull String[] permissions, @NonNull int[] grantResults) {

        if (requestCode == RC_REQUEST_PERMISSION) {
            long ptr = callbackPtr;
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                // finish starting
                callbackPtr = 0L;
                handlePermissionResult(PackageManager.PERMISSION_GRANTED, ptr);
            } else {
                callbackPtr = 0L;
                handlePermissionResult(PackageManager.PERMISSION_DENIED, ptr);
            }
        } else {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }

    public boolean isPlaying() {
        return playing;
    }

    /*
    public void startEcho(View view, TextView status_view) {

        if(playing) {
            return;
        }

        this.status_view = status_view;

        if(!createSLBufferQueueAudioPlayer()) {
            status_view.setText("Failed to create Audio Player");
            return;
        }
        // Check for record_audio permission
        int permissionCheck = ContextCompat.checkSelfPermission(getActivity(), Manifest.permission.RECORD_AUDIO);
        if (permissionCheck == PackageManager.PERMISSION_DENIED) {
            // request runtime permission
            if (ActivityCompat.shouldShowRequestPermissionRationale(getActivity(), Manifest.permission.RECORD_AUDIO)) {

                final Activity thisActivity = getActivity();
                Snackbar.make(view,
                        "This sample requires permission to record audio",
                        Snackbar.LENGTH_INDEFINITE).setAction("OK", new View.OnClickListener() {
                    @Override
                    public void onClick(View view1) {
                        requestPermissions(new String[] {Manifest.permission.RECORD_AUDIO}, RC_REQUEST_PERMISSION);
                    }
                }).show();
            } else {
                requestPermissions(new String[] {Manifest.permission.RECORD_AUDIO}, RC_REQUEST_PERMISSION);
            }


        } else {
            startEchoProcessing();
        }
    }

    */

    /**
     * Break out the method that requires the user to consent to RECORD_AUDIO.  This way
     * if the permission has been granted already, it can be started, or if the consent needs to be
     * granted, we can pick up here after the consent flow.
     */
    /*
    private void startEchoProcessing() {
        if (!createAudioRecorder()) {
            deleteSLBufferQueueAudioPlayer();
            status_view.setText("Failed to create Audio Recorder");
            return;
        }
        startPlay();   //this must include startRecording()
        playing = true;
        status_view.setText("Engine Echoing ....");
    }


    public void stopPlaying() {
        if (!playing) {
            return;
        }
        stopPlay();
        deleteSLBufferQueueAudioPlayer();
        deleteAudioRecorder();
        playing = false;
    }

*/
    /*
     * Loading our Libs
     */
    static {
        System.loadLibrary("echo");
    }

    public static native void  handlePermissionResult(int resultCode, long callbackPtr);

}
