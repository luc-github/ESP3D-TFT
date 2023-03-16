/*
  esp3d_notifications
  Copyright (c) 2022 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <mbedtls/base64.h>
#include <sys/param.h>

#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_notifications_service.h"
#include "esp3d_string.h"
#include "esp_crt_bundle.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/platform.h"
#include "mbedtls/ssl.h"
#include "network/esp3d_network.h"

#define STEP_EMAIL 0
#define STEP_ADDRESS 1
#define STEP_PORT 2
#define STEP_METHOD 3

#define EMAIL_TIMEOUT 5000

#define BUF_SIZE 512

// Email#serveraddress:port:(optional)method(SSL=default/TLS)
bool ESP3DNotificationsService::getEmailInformationsFromSettings() {
  char buffer[SIZE_OF_SETTING_NOFIFICATION_TS + 1];
  esp3dTftsettings.readString(
      ESP3DSettingIndex::esp3d_notification_token_setting, buffer,
      SIZE_OF_SETTING_NOFIFICATION_TS);
  _settings.clear();
  _serveraddress.clear();
  _port.clear();
  _method.clear();
  // 0 = email, 1 =server address, 2 = port 3 = method
  _serveraddress = "";
  uint8_t step = STEP_EMAIL;
  for (uint i = 0; i < strlen(buffer); i++) {
    if (buffer[i] == ':' || buffer[i] == '#') {
      step++;
    } else {
      switch (step) {
        case STEP_EMAIL:
          _settings += buffer[i];
          break;
        case STEP_ADDRESS:
          _serveraddress += buffer[i];
          break;
        case STEP_PORT:
          _port += buffer[i];
          break;
        case STEP_METHOD:
          _method += buffer[i];
          break;
        default:
          return false;
      }
    }
  }
  // To be backward compatible and allow no :SSL or :TLS
  if (_method.length() == 0) {
    _method = "SSL";
    step++;
  }

  if (step != 3) {
    esp3d_log_e("Missing some Parameters");
    return false;
  }
  esp3d_log("Server: %s, port: %s, email: %s, method: %s",
            _serveraddress.c_str(), _port.c_str(), _settings.c_str(),
            _method.c_str());
  return true;
}

int ESP3DNotificationsService::perform_tls_handshake(mbedtls_ssl_context *ssl) {
  int ret = -1;
  uint32_t flags;
  char *buf = NULL;
  buf = (char *)calloc(1, BUF_SIZE);
  if (buf == NULL) {
    esp3d_log_e("calloc failed for size %d", BUF_SIZE);
    return ret;
  }
  esp3d_log("Performing the SSL/TLS handshake...");
  fflush(stdout);
  int64_t startTimeout = esp3d_hal::millis();
  while ((ret = mbedtls_ssl_handshake(ssl)) != 0 &&
         ((esp3d_hal::millis() - startTimeout) < EMAIL_TIMEOUT)) {
    if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
      esp3d_log_e("mbedtls_ssl_handshake returned -0x%x", -ret);
      if (buf) {
        free(buf);
      }
      return ret;
    }
  }
  esp3d_log("Verifying peer X.509 certificate...");
  if ((flags = mbedtls_ssl_get_verify_result(ssl)) != 0) {
    /* In real life, we probably want to close connection if ret != 0 */
    esp3d_log_w("Failed to verify peer certificate!");
    mbedtls_x509_crt_verify_info(buf, BUF_SIZE, "  ! ", flags);
    esp3d_log_w("verification info: %s", buf);
  } else {
    esp3d_log("Certificate verified.");
    ret = 0;
  }
  esp3d_log("Cipher suite is %s", mbedtls_ssl_get_ciphersuite(ssl));
  if (buf) {
    free(buf);
  }
  return ret;
}

int ESP3DNotificationsService::write_ssl_and_get_response(
    mbedtls_ssl_context *ssl, unsigned char *buf, size_t len) {
  int ret;
  const size_t DATA_SIZE = 128;
  unsigned char data[DATA_SIZE];
  char code[4];
  size_t i, idx = 0;

  if (len) {
    esp3d_log("%s", buf);
  }

  while (len && (ret = mbedtls_ssl_write(ssl, buf, len)) <= 0) {
    if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
      esp3d_log_e("mbedtls_ssl_write failed with error -0x%x", -ret);
      return ret;
    }
  }
  int64_t startTimeout = esp3d_hal::millis();
  do {
    len = DATA_SIZE - 1;
    memset(data, 0, DATA_SIZE);
    ret = mbedtls_ssl_read(ssl, data, len);
    esp3d_log("ret %d", ret);
    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
      continue;
    }

    if (ret <= 0) {
      esp3d_log_e("mbedtls_ssl_read failed with error -0x%x", -ret);
      return ret;
    }

    esp3d_log("%s", data);

    len = ret;
    for (i = 0; i < len; i++) {
      if (data[i] != '\n') {
        if (idx < 4) {
          code[idx++] = data[i];
        }
        continue;
      }

      if (idx == 4 && code[0] >= '0' && code[0] <= '9' && code[3] == ' ') {
        code[3] = '\0';
        ret = atoi(code);
        return ret;
      }

      idx = 0;
    }
  } while ((esp3d_hal::millis() - startTimeout) < EMAIL_TIMEOUT);

  return ret;
}

int ESP3DNotificationsService::write_tls_and_get_response(
    mbedtls_net_context *sock_fd, unsigned char *buf, size_t len) {
  int ret;
  const size_t DATA_SIZE = 128;
  unsigned char data[DATA_SIZE];
  char code[4];
  size_t i, idx = 0;

  if (len) {
    esp3d_log("%s", buf);
  }

  if (len && (ret = mbedtls_net_send(sock_fd, buf, len)) <= 0) {
    esp3d_log_e("mbedtls_net_send failed with error -0x%x", -ret);
    return ret;
  }

  int64_t startTimeout = esp3d_hal::millis();
  do {
    len = DATA_SIZE - 1;
    memset(data, 0, DATA_SIZE);
    ret = mbedtls_net_recv(sock_fd, data, len);

    if (ret <= 0) {
      esp3d_log_e("mbedtls_net_recv failed with error -0x%x", -ret);
      return ret;
    }

    data[len] = '\0';
    printf("\n%s", data);
    len = ret;
    for (i = 0; i < len; i++) {
      if (data[i] != '\n') {
        if (idx < 4) {
          code[idx++] = data[i];
        }
        continue;
      }

      if (idx == 4 && code[0] >= '0' && code[0] <= '9' && code[3] == ' ') {
        code[3] = '\0';
        ret = atoi(code);
        return ret;
      }

      idx = 0;
    }
  } while ((esp3d_hal::millis() - startTimeout) < EMAIL_TIMEOUT);
  return ret;
}

bool ESP3DNotificationsService::sendEmailMSG(const char *title,
                                             const char *message) {
  if (_token1.length() == 0 || _token2.length() == 0 ||
      _settings.length() == 0 || _port.length() == 0 ||
      _serveraddress.length() == 0 || _method.length() == 0) {
    esp3d_log_e("Some token is missing");
    return false;
  }
  char *buf = NULL;
  unsigned char base64_buffer[128];
  int ret, len;
  size_t base64_len;
  bool hasError = false;
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_ssl_context ssl;
  mbedtls_x509_crt cacert;
  mbedtls_ssl_config conf;
  mbedtls_net_context server_fd;

  mbedtls_ssl_init(&ssl);
  mbedtls_x509_crt_init(&cacert);
  mbedtls_ctr_drbg_init(&ctr_drbg);
  esp3d_log("Seeding the random number generator");

  mbedtls_ssl_config_init(&conf);

  mbedtls_entropy_init(&entropy);
  if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                   NULL, 0)) != 0) {
    esp3d_log_e("mbedtls_ctr_drbg_seed returned -0x%x", -ret);
    hasError = true;
  }

  if (!hasError) {
    esp3d_log("Attaching the certificate bundle...");
    ret = esp_crt_bundle_attach(&conf);
    if (ret < 0) {
      esp3d_log_e("esp_crt_bundle_attach returned -0x%x\n\n", -ret);
      hasError = true;
    }
  }

  if (!hasError) {
    esp3d_log("Setting hostname for TLS session...");
    /* Hostname set here should match CN in server certificate */
    if ((ret = mbedtls_ssl_set_hostname(&ssl, _serveraddress.c_str())) != 0) {
      esp3d_log_e("mbedtls_ssl_set_hostname returned -0x%x", -ret);
      hasError = true;
    }
  }

  if (!hasError) {
    esp3d_log("Setting up the SSL/TLS structure...");

    if ((ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
      esp3d_log_e("mbedtls_ssl_config_defaults returned -0x%x", -ret);
      hasError = true;
    }
  }

  if (!hasError) {
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0) {
      esp3d_log_e("mbedtls_ssl_setup returned -0x%x", -ret);
      hasError = true;
    }
  }

  if (!hasError) {
    mbedtls_net_init(&server_fd);
    esp3d_log("Connecting to %s:%s...", _serveraddress.c_str(), _port.c_str());
    if ((ret = mbedtls_net_connect(&server_fd, _serveraddress.c_str(),
                                   _port.c_str(), MBEDTLS_NET_PROTO_TCP)) !=
        0) {
      esp3d_log_e("mbedtls_net_connect returned -0x%x", -ret);
      _lastError = ESP3DNotificationError::invalid_url;
      hasError = true;
    }
  }

  if (!hasError) {
    esp3d_log("Connected.");
    mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv,
                        NULL);
    buf = (char *)calloc(1, BUF_SIZE);
    if (buf == NULL) {
      esp3d_log_e("calloc failed for size %d", BUF_SIZE);
      hasError = true;
    }
  }

  if (!hasError) {
    if (_method == "TLS") {
      /* Get response */
      ret = write_tls_and_get_response(&server_fd, (unsigned char *)buf, 0);
      if (ret < 200 || ret > 299) {
        esp3d_log_e("Failed to get proper response");
        hasError = true;
      }
      if (!hasError) {
        esp3d_log("Writing EHLO to server...");
        len = snprintf((char *)buf, BUF_SIZE, "EHLO %s\r\n", "ESP32");
        ret = write_tls_and_get_response(&server_fd, (unsigned char *)buf, len);
        if (ret < 200 || ret > 299) {
          esp3d_log_e("Failed to get proper response");
          hasError = true;
        }
      }
      if (!hasError) {
        esp3d_log("Writing STARTTLS to server...");
        len = snprintf((char *)buf, BUF_SIZE, "STARTTLS\r\n");
        ret = write_tls_and_get_response(&server_fd, (unsigned char *)buf, len);
        if (ret < 200 || ret > 299) {
          esp3d_log_e("Failed to get proper response");
          hasError = true;
        }
      }
      if (!hasError) {
        ret = perform_tls_handshake(&ssl);
        if (ret != 0) {
          esp3d_log_e("perform_tls_handshake failed");
          hasError = true;
        }
      }
    } else { /* Not SERVER_USES_STARTSSL */
      ret = perform_tls_handshake(&ssl);
      if (ret != 0) {
        esp3d_log_e("perform_tls_handshake failed");
        hasError = true;
      }
      if (!hasError) {
        /* Get response */
        ret = write_ssl_and_get_response(&ssl, (unsigned char *)buf, 0);
        if (ret < 200 || ret > 299) {
          esp3d_log_e("Failed to get proper response");
          hasError = true;
        }
      }

      if (!hasError) {
        esp3d_log("Writing EHLO to server...");
        len = snprintf((char *)buf, BUF_SIZE, "EHLO %s\r\n", "ESP32");
        ret = write_ssl_and_get_response(&ssl, (unsigned char *)buf, len);
        if (ret < 200 || ret > 299) {
          esp3d_log_e("Failed to get proper response");
          hasError = true;
        }
      }
    }
  }

  if (!hasError) {
    esp3d_log("Authentication...");
    esp3d_log("Write AUTH LOGIN");
    len = snprintf((char *)buf, BUF_SIZE, "AUTH LOGIN\r\n");
    ret = write_ssl_and_get_response(&ssl, (unsigned char *)buf, len);
    if (ret < 200 || ret > 399) {
      esp3d_log_e("Failed to get proper response");
      hasError = true;
    }
  }
  if (!hasError) {
    esp3d_log("Write USER NAME %s", _token1.c_str());
    ret = mbedtls_base64_encode(
        (unsigned char *)base64_buffer, sizeof(base64_buffer), &base64_len,
        (unsigned char *)_token1.c_str(), strlen(_token1.c_str()));
    if (ret != 0) {
      esp3d_log_e("Error in mbedtls encode! ret = -0x%x", -ret);
      hasError = true;
    }
  }

  if (!hasError) {
    len = snprintf((char *)buf, BUF_SIZE, "%s\r\n", base64_buffer);
    ret = write_ssl_and_get_response(&ssl, (unsigned char *)buf, len);
    if (ret < 300 || ret > 399) {
      esp3d_log_e("Failed to get proper response");
      _lastError = ESP3DNotificationError::invalid_token1;
      hasError = true;
    }
  }

  if (!hasError) {
    esp3d_log("Write PASSWORD");
    ret = mbedtls_base64_encode(
        (unsigned char *)base64_buffer, sizeof(base64_buffer), &base64_len,
        (unsigned char *)_token2.c_str(), strlen(_token2.c_str()));
    if (ret != 0) {
      esp3d_log_e("Error in mbedtls encode! ret = -0x%x", -ret);
      hasError = true;
    }
  }

  if (!hasError) {
    len = snprintf((char *)buf, BUF_SIZE, "%s\r\n", base64_buffer);
    ret = write_ssl_and_get_response(&ssl, (unsigned char *)buf, len);
    if (ret < 200 || ret > 399) {
      esp3d_log_e("Failed to get proper response");
      _lastError = ESP3DNotificationError::invalid_token2;
      hasError = true;
    }
  }

  if (!hasError) {
    /* Compose email */
    esp3d_log("Write MAIL FROM %s", _token1.c_str());
    len =
        snprintf((char *)buf, BUF_SIZE, "MAIL FROM:<%s>\r\n", _token1.c_str());
    ret = write_ssl_and_get_response(&ssl, (unsigned char *)buf, len);
    if (ret < 200 || ret > 299) {
      esp3d_log_e("Failed to get proper response");
      hasError = true;
    }
  }
  if (!hasError) {
    esp3d_log("Write RCPT %s", _settings.c_str());
    len =
        snprintf((char *)buf, BUF_SIZE, "RCPT TO:<%s>\r\n", _settings.c_str());
    ret = write_ssl_and_get_response(&ssl, (unsigned char *)buf, len);
    if (ret < 200 || ret > 299) {
      esp3d_log_e("Failed to get proper response");
      hasError = true;
    }
  }

  if (!hasError) {
    esp3d_log("Write DATA");
    len = snprintf((char *)buf, BUF_SIZE, "DATA\r\n");
    ret = write_ssl_and_get_response(&ssl, (unsigned char *)buf, len);
    if (ret < 300 || ret > 399) {
      esp3d_log_e("Failed to get proper response");
      hasError = true;
    }
  }

  if (!hasError) {
    /* We do not take action if message sending is partly failed. */
    std::string messageFormat = "From:";
    messageFormat += esp3dNetwork.getHostName();
    messageFormat += "\r\n";
    messageFormat += "To: <%s>\r\nSubject:";
    messageFormat += title;
    messageFormat += "\r\n\r\n";
    messageFormat += message;
    messageFormat += "\r\n.\r\n";
    len = snprintf((char *)buf, BUF_SIZE, messageFormat.c_str(),
                   _token1.c_str(), _settings.c_str());
    esp3d_log("Write email Content");
    ret = write_ssl_and_get_response(&ssl, (unsigned char *)buf, len);
    if (ret < 200 || ret > 299) {
      esp3d_log_e("Failed to get proper response");
      hasError = true;
      _lastError = ESP3DNotificationError::invalid_data;
    }
  }

  if (!hasError) {
    esp3d_log("Email sent!");
    // Quit
    len = snprintf((char *)buf, BUF_SIZE, "QUIT\r\n");
    ret = write_ssl_and_get_response(&ssl, (unsigned char *)buf, len);
    if (ret < 200 || ret > 299) {
      esp3d_log_e("Failed to get proper response");
      hasError = true;
    }
  }

  if (!hasError) {
    /* Close connection */
    mbedtls_ssl_close_notify(&ssl);
    _lastError = ESP3DNotificationError::no_error;
  } else {
    if (_lastError == ESP3DNotificationError::no_error) {
      _lastError = ESP3DNotificationError::error;
    }
  }

  mbedtls_net_free(&server_fd);
  mbedtls_x509_crt_free(&cacert);
  mbedtls_ssl_free(&ssl);
  mbedtls_ssl_config_free(&conf);
  mbedtls_ctr_drbg_free(&ctr_drbg);
  mbedtls_entropy_free(&entropy);
  if (buf) {
    free(buf);
  }
  return !hasError;
}
