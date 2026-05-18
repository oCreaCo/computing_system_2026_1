#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>

#define N_THREADS 8
#define LOG_SIZE (1024 * 1024 * 1024)
#define LOG_LINE_SIZE 128

_Thread_local char log_line[LOG_LINE_SIZE];

static int rint_range(int a, int b) { // [a, b]
    return a + rand() % (b - a + 1);
}
static double rdouble_range(double a, double b) {
    return a + ((double)rand() / (double)RAND_MAX) * (b - a);
}

static const char *names[] = {"alice","bob","charlie","david","erin","frank","grace","henry","ivy","jack","kim","lucas","mina","noah","olivia","peter","queen","ryan","sara","tom"};
static const char *domains[] = {"example.com","mail.com","domain.com","service.io","corp.internal"};
static const char *topics[] = {"user_events","order_events","audit_logs","billing","alerts"};
static const char *upstreams[] = {"inventory-service","billing-api","payment-gateway","profile-service","shipping-api"};
static const char *jobs[] = {"EmailSender","DailyReportJob","SessionCleanup","FeedRefresher","ReindexSearch"};
static const char *buckets[] = {"user-media","attachments","exports","reports","static-assets"};

static void rand_email(char *buf, size_t n) {
    snprintf(buf, n, "%s@%s", names[rint_range(0,(int)(sizeof(names)/sizeof(names[0]))-1)],
             domains[rint_range(0,(int)(sizeof(domains)/sizeof(domains[0]))-1)]);
}
static void rand_ip(char *buf, size_t n) {
    snprintf(buf, n, "%d.%d.%d.%d", rint_range(1,223), rint_range(0,255), rint_range(0,255), rint_range(1,254));
}
static void rand_trace(char *buf, size_t n) {
    snprintf(buf, n, "%x%x%x", rint_range(0,0xffff), rint_range(0,0xffff), rint_range(0,0xffff));
}
static void rand_version(char *buf, size_t n) {
    snprintf(buf, n, "%d.%d.%d", rint_range(1,3), rint_range(0,9), rint_range(0,9));
}

typedef struct { const char *fmt; } Tmpl;

static Tmpl INFO_TMPL[] = {
    {"Connected to database '%s' as user '%s'"},
    {"Health check endpoint responded in %.1fms"},
    {"User '%s' logged in successfully"},
    {"New user registered: id=%d email='%s'"},
    {"Loaded %d active feature flags"},
    {"Cache warmed with %d user profiles"},
    {"Session cleanup completed, removed %d expired sessions"},
    {"Background job '%s' executed successfully"},
    {"Configuration reloaded (version=%s)"},
    {"Token refresh succeeded for user_id=%d"},
    {"WebSocket connection established (client_id=%s)"},
    {"API request GET /v1/products completed in %dms"},
    {"Created order #%d for user_id=%d"},
    {"Payment service returned status OK"},
    {"Scheduled task '%s' started"},
    {"Scheduled task '%s' completed successfully"},
    {"Uploaded file 'invoice_%d.pdf' (size=%dKB)"},
    {"Email sent to '%s'"},
    {"Redis connection established at %s:%d"},
    {"Connected to upstream API '%s'"},
    {"Metrics exporter initialized on port %d"},
    {"TLS handshake completed with client %s"},
    {"Service health status: OK"},
    {"Memory usage stable (RSS=%dMB)"},
    {"API version %s deployed successfully"},
    {"Connection pool initialized (size=%d)"},
    {"Backup completed successfully (duration=%.1fs)"},
    {"User '%s' updated profile picture"},
    {"Message queued to Kafka topic '%s'"},
    {"Data sync with remote node finished (delta=%d records)"},
    {"Database migration applied: V%d__add_payment_table.sql"},
    {"Started processing batch job id=%d"},
    {"Batch job id=%d completed successfully"},
    {"Generated access token for client_id=%s"},
    {"JWT validation succeeded for request /v1/orders"},
    {"Received %d requests in last minute (avg latency=%dms)"},
    {"Loaded configuration from environment variables"},
    {"Uploaded %d new images to S3 bucket '%s'"},
    {"File download completed: /tmp/report_%d.xlsx"},
    {"User '%s' logged out"},
    {"Session token revoked (user_id=%d)"},
    {"Added %d new nodes to cluster"},
    {"Replication sync with node %s completed"},
    {"Log rotation triggered, archive created: server.log.%d"},
    {"Cache size reduced from %dMB to %dMB"},
    {"Background worker pool started (threads=%d)"},
    {"Application uptime: %dh %dm %ds"}
};

static Tmpl DEBUG_TMPL[] = {
    {"Parsing request headers for trace_id=%s"},
    {"Executing SQL query: SELECT * FROM orders WHERE user_id=%d"},
    {"JSON serialization of response payload started (size=%d bytes)"},
    {"HTTP handler /v1/users invoked with params {limit:%d}"},
    {"Acquired database connection from pool (id=%d)"},
    {"Released database connection back to pool"},
    {"Generating JWT for user_id=%d"},
    {"Validating incoming token signature"},
    {"Cache lookup hit for key 'user:profile:%d'"},
    {"Cache lookup miss for key 'session:token:%s'"},
    {"Retrying connection to Redis (attempt=%d)"},
    {"Received POST payload (size=%d bytes)"},
    {"Response compressed using gzip (ratio=%.2f)"},
    {"Deserialized JSON body successfully"},
    {"Dispatching async task '%s'"},
    {"Reconnecting to upstream service %s"},
    {"HTTP client connection reused (keep-alive)"},
    {"Processing queue item id=%d"},
    {"Request latency histogram updated (bucket=%dms)"},
    {"Marshaling order entity to protobuf"},
    {"Prepared statement cached: %s"},
    {"Thread worker-%d idle, waiting for next job"},
    {"GC cycle completed, freed %dKB"},
    {"Updated in-memory session map (count=%d)"},
    {"Cleaning expired temp files older than %dh"}
};

static Tmpl WARNING_TMPL[] = {
    {"Slow API response: /v1/orders took %dms"},
    {"Redis connection retry #%d after timeout"},
    {"Deprecated endpoint /v1/profile accessed by client v%s"},
    {"Missing required header 'X-Trace-Id' in request"},
    {"User session approaching expiration (id=%s)"},
    {"JWT expiring soon (user_id=%d, exp=%ds)"},
    {"External API '%s' returned 429 Too Many Requests"},
    {"Request rate exceeded limit for IP %s"},
    {"Background worker pool utilization exceeded %d%%"},
    {"Temporary disk space low (remaining=%d%%)"}
};

static Tmpl ERROR_TMPL[] = {
    {"Database connection lost: timeout after %ds"},
    {"JSON parse error in POST /v1/products: invalid character at position %d"},
    {"Authentication failed for user '%s': invalid credentials"},
    {"Failed to write log file: No space left on device"},
    {"Upstream service '%s' returned 500 Internal Server Error"}
};

void set_info_log()
{
    int rnd = rand() % 48;
    int len = 10;
    char email[64], ip[32], trace[32], version[16];

    sprintf(log_line, "[INFO]    ");

    switch (rnd) {
    case 0:
        sprintf(log_line + len, INFO_TMPL[0].fmt, "users_db", names[rand() % 20]);
        break;
    case 1:
        sprintf(log_line + len, INFO_TMPL[1].fmt, rdouble_range(1.0, 100.0));
        break;
    case 2:
        sprintf(log_line + len, INFO_TMPL[2].fmt, names[rand() % 20]);
        break;
    case 3:
        rand_email(email, sizeof(email));
        sprintf(log_line + len, INFO_TMPL[3].fmt, rint_range(1000, 9999), email);
        break;
    case 4:
        sprintf(log_line + len, INFO_TMPL[4].fmt, rint_range(5, 50));
        break;
    case 5:
        sprintf(log_line + len, INFO_TMPL[5].fmt, rint_range(100, 10000));
        break;
    case 6:
        sprintf(log_line + len, INFO_TMPL[6].fmt, rint_range(1, 500));
        break;
    case 7:
        sprintf(log_line + len, INFO_TMPL[7].fmt, jobs[rand() % 5]);
        break;
    case 8:
        rand_version(version, sizeof(version));
        sprintf(log_line + len, INFO_TMPL[8].fmt, version);
        break;
    case 9:
        sprintf(log_line + len, INFO_TMPL[9].fmt, rint_range(1000, 9999));
        break;
    case 10:
        rand_trace(trace, sizeof(trace));
        sprintf(log_line + len, INFO_TMPL[10].fmt, trace);
        break;
    case 11:
        sprintf(log_line + len, INFO_TMPL[11].fmt, rint_range(10, 500));
        break;
    case 12:
        sprintf(log_line + len, INFO_TMPL[12].fmt, rint_range(1000, 9999), rint_range(1000, 9999));
        break;
    case 13:
        sprintf(log_line + len, INFO_TMPL[13].fmt);
        break;
    case 14:
        sprintf(log_line + len, INFO_TMPL[14].fmt, jobs[rand() % 5]);
        break;
    case 15:
        sprintf(log_line + len, INFO_TMPL[15].fmt, jobs[rand() % 5]);
        break;
    case 16:
        sprintf(log_line + len, INFO_TMPL[16].fmt, rint_range(1000, 9999), rint_range(10, 1000));
        break;
    case 17:
        rand_email(email, sizeof(email));
        sprintf(log_line + len, INFO_TMPL[17].fmt, email);
        break;
    case 18:
        rand_ip(ip, sizeof(ip));
        sprintf(log_line + len, INFO_TMPL[18].fmt, ip, rint_range(6379, 6399));
        break;
    case 19:
        sprintf(log_line + len, INFO_TMPL[19].fmt, upstreams[rand() % 5]);
        break;
    case 20:
        sprintf(log_line + len, INFO_TMPL[20].fmt, rint_range(8000, 9000));
        break;
    case 21:
        rand_ip(ip, sizeof(ip));
        sprintf(log_line + len, INFO_TMPL[21].fmt, ip);
        break;
    case 22:
        sprintf(log_line + len, INFO_TMPL[22].fmt);
        break;
    case 23:
        sprintf(log_line + len, INFO_TMPL[23].fmt, rint_range(100, 2000));
        break;
    case 24:
        rand_version(version, sizeof(version));
        sprintf(log_line + len, INFO_TMPL[24].fmt, version);
        break;
    case 25:
        sprintf(log_line + len, INFO_TMPL[25].fmt, rint_range(5, 50));
        break;
    case 26:
        sprintf(log_line + len, INFO_TMPL[26].fmt, rdouble_range(1.0, 60.0));
        break;
    case 27:
        sprintf(log_line + len, INFO_TMPL[27].fmt, names[rand() % 20]);
        break;
    case 28:
        sprintf(log_line + len, INFO_TMPL[28].fmt, topics[rand() % 5]);
        break;
    case 29:
        sprintf(log_line + len, INFO_TMPL[29].fmt, rint_range(1, 1000));
        break;
    case 30:
        sprintf(log_line + len, INFO_TMPL[30].fmt, rint_range(1, 100));
        break;
    case 31:
        sprintf(log_line + len, INFO_TMPL[31].fmt, rint_range(1000, 9999));
        break;
    case 32:
        sprintf(log_line + len, INFO_TMPL[32].fmt, rint_range(1000, 9999));
        break;
    case 33:
        rand_trace(trace, sizeof(trace));
        sprintf(log_line + len, INFO_TMPL[33].fmt, trace);
        break;
    case 34:
        sprintf(log_line + len, INFO_TMPL[34].fmt);
        break;
    case 35:
        sprintf(log_line + len, INFO_TMPL[35].fmt, rint_range(100, 10000), rint_range(10, 500));
        break;
    case 36:
        sprintf(log_line + len, INFO_TMPL[36].fmt);
        break;
    case 37:
        sprintf(log_line + len, INFO_TMPL[37].fmt, rint_range(1, 100), buckets[rand() % 5]);
        break;
    case 38:
        sprintf(log_line + len, INFO_TMPL[38].fmt, rint_range(1000, 9999));
        break;
    case 39:
        sprintf(log_line + len, INFO_TMPL[39].fmt, names[rand() % 20]);
        break;
    case 40:
        sprintf(log_line + len, INFO_TMPL[40].fmt, rint_range(1000, 9999));
        break;
    case 41:
        sprintf(log_line + len, INFO_TMPL[41].fmt, rint_range(1, 10));
        break;
    case 42:
        rand_ip(ip, sizeof(ip));
        sprintf(log_line + len, INFO_TMPL[42].fmt, ip);
        break;
    case 43:
        sprintf(log_line + len, INFO_TMPL[43].fmt, rint_range(1, 100));
        break;
    case 44:
        sprintf(log_line + len, INFO_TMPL[44].fmt, rint_range(100, 500), rint_range(50, 200));
        break;
    case 45:
        sprintf(log_line + len, INFO_TMPL[45].fmt, rint_range(4, 32));
        break;
    case 46:
        sprintf(log_line + len, INFO_TMPL[46].fmt, rint_range(0, 100), rint_range(0, 59), rint_range(0, 59));
        break;
    default:
        sprintf(log_line + len, INFO_TMPL[13].fmt);
        break;
    }

    int current_len = strlen(log_line);
    for (int i = current_len; i < LOG_LINE_SIZE - 1; i++) {
        log_line[i] = ' ';
    }
    log_line[LOG_LINE_SIZE - 1] = '\n';
}

void set_debug_log()
{
    int rnd = rand() % 25;
    int len = 10;
    char trace[32], version[16];

    sprintf(log_line, "[DEBUG]   ");

    switch (rnd) {
    case 0:
        rand_trace(trace, sizeof(trace));
        sprintf(log_line + len, DEBUG_TMPL[0].fmt, trace);
        break;
    case 1:
        sprintf(log_line + len, DEBUG_TMPL[1].fmt, rint_range(1000, 9999));
        break;
    case 2:
        sprintf(log_line + len, DEBUG_TMPL[2].fmt, rint_range(100, 10000));
        break;
    case 3:
        sprintf(log_line + len, DEBUG_TMPL[3].fmt, rint_range(10, 100));
        break;
    case 4:
        sprintf(log_line + len, DEBUG_TMPL[4].fmt, rint_range(1, 50));
        break;
    case 5:
        sprintf(log_line + len, DEBUG_TMPL[5].fmt);
        break;
    case 6:
        sprintf(log_line + len, DEBUG_TMPL[6].fmt, rint_range(1000, 9999));
        break;
    case 7:
        sprintf(log_line + len, DEBUG_TMPL[7].fmt);
        break;
    case 8:
        sprintf(log_line + len, DEBUG_TMPL[8].fmt, rint_range(1000, 9999));
        break;
    case 9:
        rand_trace(trace, sizeof(trace));
        sprintf(log_line + len, DEBUG_TMPL[9].fmt, trace);
        break;
    case 10:
        sprintf(log_line + len, DEBUG_TMPL[10].fmt, rint_range(1, 5));
        break;
    case 11:
        sprintf(log_line + len, DEBUG_TMPL[11].fmt, rint_range(100, 10000));
        break;
    case 12:
        sprintf(log_line + len, DEBUG_TMPL[12].fmt, rdouble_range(1.5, 5.0));
        break;
    case 13:
        sprintf(log_line + len, DEBUG_TMPL[13].fmt);
        break;
    case 14:
        sprintf(log_line + len, DEBUG_TMPL[14].fmt, jobs[rand() % 5]);
        break;
    case 15:
        sprintf(log_line + len, DEBUG_TMPL[15].fmt, upstreams[rand() % 5]);
        break;
    case 16:
        sprintf(log_line + len, DEBUG_TMPL[16].fmt);
        break;
    case 17:
        sprintf(log_line + len, DEBUG_TMPL[17].fmt, rint_range(1000, 9999));
        break;
    case 18:
        sprintf(log_line + len, DEBUG_TMPL[18].fmt, rint_range(10, 500));
        break;
    case 19:
        sprintf(log_line + len, DEBUG_TMPL[19].fmt);
        break;
    case 20:
        sprintf(log_line + len, DEBUG_TMPL[20].fmt, "SELECT * FROM users WHERE id = ?");
        break;
    case 21:
        sprintf(log_line + len, DEBUG_TMPL[21].fmt, rint_range(1, 16));
        break;
    case 22:
        sprintf(log_line + len, DEBUG_TMPL[22].fmt, rint_range(100, 10000));
        break;
    case 23:
        sprintf(log_line + len, DEBUG_TMPL[23].fmt, rint_range(1, 1000));
        break;
    case 24:
        sprintf(log_line + len, DEBUG_TMPL[24].fmt, rint_range(1, 24));
        break;
    default:
        sprintf(log_line + len, DEBUG_TMPL[13].fmt);
        break;
    }

    int current_len = strlen(log_line);
    // printf("rnd: %d, log_line: %s, current_len: %d\n", rnd, log_line, current_len);
    for (int i = current_len; i < LOG_LINE_SIZE - 1; i++) {
        log_line[i] = ' ';
    }
    log_line[LOG_LINE_SIZE - 1] = '\n';
}

void set_warning_log()
{
    int rnd = rand() % 10;
    int len = 10;
    char version[16], trace[32], ip[32];

    sprintf(log_line, "[WARNING] ");

    switch (rnd) {
    case 0:
        sprintf(log_line + len, WARNING_TMPL[0].fmt, rint_range(500, 5000));
        break;
    case 1:
        sprintf(log_line + len, WARNING_TMPL[1].fmt, rint_range(1, 5));
        break;
    case 2:
        rand_version(version, sizeof(version));
        sprintf(log_line + len, WARNING_TMPL[2].fmt, version);
        break;
    case 3:
        sprintf(log_line + len, WARNING_TMPL[3].fmt);
        break;
    case 4:
        rand_trace(trace, sizeof(trace));
        sprintf(log_line + len, WARNING_TMPL[4].fmt, trace);
        break;
    case 5:
        sprintf(log_line + len, WARNING_TMPL[5].fmt, rint_range(1000, 9999), rint_range(60, 3600));
        break;
    case 6:
        sprintf(log_line + len, WARNING_TMPL[6].fmt, upstreams[rand() % 5]);
        break;
    case 7:
        rand_ip(ip, sizeof(ip));
        sprintf(log_line + len, WARNING_TMPL[7].fmt, ip);
        break;
    case 8:
        sprintf(log_line + len, WARNING_TMPL[8].fmt, rint_range(80, 95));
        break;
    case 9:
        sprintf(log_line + len, WARNING_TMPL[9].fmt, rint_range(5, 20));
        break;
    default:
        sprintf(log_line + len, WARNING_TMPL[3].fmt);
        break;
    }

    int current_len = strlen(log_line);
    for (int i = current_len; i < LOG_LINE_SIZE - 1; i++) {
        log_line[i] = ' ';
    }
    log_line[LOG_LINE_SIZE - 1] = '\n';
}

void set_error_log()
{
    int rnd = rand() % 5;
    int len = 10;

    sprintf(log_line, "[ERROR]   ");

    switch (rnd) {
    case 0:
        sprintf(log_line + len, ERROR_TMPL[0].fmt, rint_range(10, 60));
        break;
    case 1:
        sprintf(log_line + len, ERROR_TMPL[1].fmt, rint_range(1, 1000));
        break;
    case 2:
        sprintf(log_line + len, ERROR_TMPL[2].fmt, names[rand() % 20]);
        break;
    case 3:
        sprintf(log_line + len, ERROR_TMPL[3].fmt);
        break;
    case 4:
        sprintf(log_line + len, ERROR_TMPL[4].fmt, upstreams[rand() % 5]);
        break;
    default:
        sprintf(log_line + len, ERROR_TMPL[3].fmt);
        break;
    }

    int current_len = strlen(log_line);
    for (int i = current_len; i < LOG_LINE_SIZE - 1; i++) {
        log_line[i] = ' ';
    }
    log_line[LOG_LINE_SIZE - 1] = '\n';
}

void *thread_func(void *arg)
{
    int thread_id = (int)(arg);
    int log_size_per_thr = LOG_SIZE / N_THREADS;
    int offset = log_size_per_thr * thread_id;
    int total_size = 0;
    int fd, rnd;

    fd = open("server.log", O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        perror("file open failed");
        return 1;
    }

    while (total_size < log_size_per_thr) {
        rnd = rand() % 1111;

        if (rnd < 1000)
            set_info_log();
        else if (rnd < 1100)
            set_debug_log();
        else if (rnd < 1110)
            set_warning_log();
        else if (rnd < 1111)
            set_error_log();
        else
            return 1;

        pwrite(fd, log_line, LOG_LINE_SIZE, offset + total_size);
        total_size += LOG_LINE_SIZE;
    }

    close(fd);

    return (void*)NULL;
}

int main(void)
{
    pthread_t threads[N_THREADS];
    srand(time(NULL));
    
    for (int i = 0; i < N_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_func, (void*)i);
    }

    for (int i = 0; i < N_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
