#include "ytjssinglevideosource.h"

#include "js.h"
#include "video.h"

YTJSSingleVideoSource::YTJSSingleVideoSource(QObject *parent)
    : VideoSource(parent), video(nullptr) {}

void YTJSSingleVideoSource::loadVideos(int max, int startIndex) {
    aborted = false;

    if (startIndex == 1) {
        if (video) {
            if (name.isEmpty()) {
                name = video->getTitle();
                qDebug() << "Emitting name changed" << name;
                emit nameChanged(name);
            }
            emit gotVideos({video->clone()});
        }
    }

    JS::instance()
            .callFunction(new JSResult(this), "videoInfo", {videoId})
            .onJson([this](auto &doc) {
                if (aborted) return;

                auto obj = doc.object();
                // qDebug() << doc.toJson();

                const auto items = obj["related_videos"].toArray();
                QVector<Video *> videos;
                videos.reserve(items.size());

                for (const auto &i : items) {
                    Video *video = new Video();

                    QString id = i["id"].toString();
                    video->setId(id);

                    QString title = i["title"].toString();
                    video->setTitle(title);

                    QString desc = i["description"].toString();
                    if (desc.isEmpty()) desc = i["desc"].toString();
                    video->setDescription(desc);

                    QString thumb = i["video_thumbnail"].toString();
                    video->setThumbnailUrl(thumb);

                    int views = i["view_count"].toInt();
                    video->setViewCount(views);

                    int duration = i["length_seconds"].toInt();
                    video->setViewCount(duration);

                    QString channelId = i["ucid"].toString();
                    video->setChannelId(channelId);

                    QString channelName = i["author"].toString();
                    video->setChannelTitle(channelName);

                    videos << video;
                }

                if (videos.isEmpty()) {
                    emit error("No results");
                } else {
                    emit gotVideos(videos);
                    emit finished(videos.size());
                }

                // fake more videos by loading videos related to the last one
                video = nullptr;
                if (!videos.isEmpty()) videoId = videos.last()->getId();
            })
            .onError([this](auto &msg) { emit error(msg); });
}

void YTJSSingleVideoSource::setVideo(Video *video) {
    this->video = video;
    videoId = video->getId();
}

void YTJSSingleVideoSource::abort() {
    aborted = true;
}

QString YTJSSingleVideoSource::getName() {
    return name;
}
