package org.qtproject.example;

import android.content.ContentValues;
import android.content.Context;
import android.net.Uri;
import android.os.Environment;
import android.provider.MediaStore;
import java.io.OutputStream;

public class ImageSaver {
    public static String saveImageToGallery(Context context, byte[] imageData, String filename, String mimeType) {
        try {
            ContentValues values = new ContentValues();
            values.put(MediaStore.Images.Media.DISPLAY_NAME, filename);
            values.put(MediaStore.Images.Media.MIME_TYPE, mimeType);
            values.put(MediaStore.Images.Media.RELATIVE_PATH, Environment.DIRECTORY_PICTURES + "/MyApp");

            Uri uri = context.getContentResolver().insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, values);
            if (uri == null) return null;

            OutputStream outputStream = context.getContentResolver().openOutputStream(uri);
            if (outputStream == null) return null;

            outputStream.write(imageData);
            outputStream.close();

            return uri.toString();  // 返回图片保存的路径
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
}
