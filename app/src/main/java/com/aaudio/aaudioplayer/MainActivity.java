package com.aaudio.aaudioplayer;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;
import com.aaudio.aaudioplayer.databinding.ActivityMainBinding;
import androidx.core.content.ContextCompat;
import android.content.pm.PackageManager;
import android.widget.Toast;
import android.Manifest;

import androidx.core.app.ActivityCompat;

public class MainActivity extends AppCompatActivity {

    // 네이티브 라이브러리 로드
    static {
        System.loadLibrary("aaudioplayer");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_MEDIA_AUDIO)
                != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.READ_MEDIA_AUDIO}, 1);
        }

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // TextView 객체 설정
        TextView tv = binding.sampleText;
        tv.setText("Press Play to start playback");

        // 버튼 객체 설정
        Button playButton = binding.playButton;
        Button stopButton = binding.stopButton;
        Button savePcmButton = binding.savePcmButton;

        // 버튼 클릭 이벤트
        playButton.setOnClickListener(view -> startPlayback());
        stopButton.setOnClickListener(view -> stopPlayback());
        savePcmButton.setOnClickListener(view -> savePCMToFile());
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == 1) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                // 권한이 승인되면 음원 재생 준비
            } else {
                // 권한이 거부된 경우 오류 처리
                Toast.makeText(this, "Audio permission is required to play the file.", Toast.LENGTH_SHORT).show();
            }
        }
    }

    // 네이티브 메서드 정의
    public native void startPlayback();
    public native void stopPlayback();
    public native void savePCMToFile();
}
